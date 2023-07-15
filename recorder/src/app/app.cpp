#include "app.h"

#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <tools/MutexQueue.h>

#include <filesystem>
#include <memory>
#include <ranges>
#include <string>
#include <thread>
#include <vector>

#include "app/config.h"
#include "sag/match/MatchRecorder.h"
#include "sag/mcts/MCTS.h"
#include "sag/mcts/MCTSPlayer.h"
#include "sag/santorini/Graph.h"
#include "sag/santorini/Santorini.h"
#include "sag/storage/SQLiteMatchStorage.h"
#include "tools/BoundedValue.h"
#include "tools/Literals.h"

using namespace tools::literals::memory;

namespace {
struct ReadCommandLoop {
	std::istream& command_source;  // NOLINT(*const-or-ref-data-members)
	std::shared_ptr<spdlog::logger> logger;

	auto operator()(std::stop_token const& token, tools::MutexQueue<std::string>* queue) -> void {
		logger->info("thread loop starts");
		while (!token.stop_requested()) {
			std::string input;
			logger->debug("waiting for user input");
			std::getline(command_source, input);
			logger->trace("received user input: {}", input);
			queue->emplace(input);

			// convert to lower case before comparing to exit conditions
			std::ranges::transform(
				input, input.begin(), [](unsigned char letter) { return static_cast<char>(std::tolower(letter)); });
			if (input == "q" || input == "quit" || input == "exit") {
				logger->info("exit requested (input: {0})", input);
				break;
			}
		}
		logger->info("thread loop ends");
	}
};
}  // namespace

namespace app {

namespace {
auto create_logger(std::string const& name, config::Log const& log_config) -> spdlog::logger {
	std::vector<spdlog::sink_ptr> sinks;

	if (auto cfg = log_config.console; cfg.has_value()) {
		auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		sink->set_level(cfg->level);
		sink->set_pattern(cfg->pattern);
		sinks.emplace_back(sink);
	}

	if (auto cfg = log_config.file; cfg.has_value()) {
		if (!std::filesystem::is_directory(cfg->folder))
			throw std::runtime_error(fmt::format("failed to create logger: '{}' is not a directory", cfg->folder));

		std::filesystem::path log_file = fmt::format("{}/{}.log", cfg->folder, name);
		auto sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
			log_file.string(), cfg->max_size_mb * 1_MiB, cfg->max_files);
		sink->set_level(cfg->level);
		sink->set_pattern(cfg->pattern);
		sinks.emplace_back(sink);
	}

	return spdlog::logger{name, sinks.begin(), sinks.end()};
}
}  // namespace

auto App::run(config::Recorder const& config) -> int {
	auto logger = std::make_shared<spdlog::logger>(create_logger("app", config.log));
	// spdlog::set_default_logger(logger);

	try {
		using enum sag::match::Signal;
		const std::map<std::string, sag::match::Signal, std::less<>> cliRecorderSignals = {
			{"record", Record},
			{"r", Record},
			{"halt", Halt},
			{"h", Halt},
			{"quit", Quit},
			{"q", Quit},
			{"exit", Quit},
			{"status", Status},
			{"s", Status},
			{"info", Status},
		};
		logger->info("app start");

		tools::SingleQueuedThreadHandle<std::string> cli_thread(ReadCommandLoop{input_source_, logger});

		// prepare and inject the game-sepcific types
		constexpr sag::santorini::Dimensions dim =  // avoid using 5x5x2, to keep app tests short
			{.rows = 3, .cols = 3, .player_unit_count = 1};
		using TGraph = sag::santorini::Graph<dim>;
		using TStorage =
			sag::storage::SQLiteMatchStorage<sag::santorini::StateConverter<dim>, sag::santorini::ActionConverter>;
		using TRec = sag::match::MatchRecorder<TGraph, TStorage>;

		// validate storage db file path exists
		std::string_view const db_file_path = config.db_file_path;
		if (std::filesystem::exists(db_file_path) && !std::filesystem::is_regular_file(db_file_path) &&
				!std::filesystem::is_symlink(db_file_path)) {
			logger->error("Existing object at path '{}' is neither a file nor a symlink", config.db_file_path);
			return 1;
		}

		// prepare and start recorder threads acc. to configuration
		std::vector<sag::match::RecorderThreadHandle<TRec>> recorder_threads;
		recorder_threads.reserve(config.parallel_games);
		for (size_t i = 0; i < config.parallel_games; ++i) {
			std::vector<std::unique_ptr<sag::match::Player<TGraph>>> players;
			players.reserve(config.players.size());

			for (config::Player const& player_config : config.players) {
				players.emplace_back(std::make_unique<sag::mcts::MCTSPlayer<TGraph>>(player_config.name,
					player_config.mcts.simulations,
					tools::NonNegative{1.0F},
					player_config.name,
					sag::mcts::BaseMCTS<TGraph>{
						player_config.mcts.sample_uniformly, tools::NonNegative{player_config.mcts.explore_constant}}));
			}

			auto recorder_logger = std::make_shared<spdlog::logger>(create_logger(fmt::format("rec-{}", i), config.log));
			auto sql_connection = std::make_unique<tools::SQLiteConnection>(db_file_path, false, recorder_logger);
			TStorage storage{std::move(sql_connection)};
			TRec recorder{std::move(players), {}, {}, std::move(storage), std::move(recorder_logger)};

			recorder_threads.emplace_back(std::move(recorder));
		}

		while (true) {
			std::string command = cli_thread.queue().wait_and_dequeue();
			std::ranges::transform(
				command, command.begin(), [](unsigned char letter) { return static_cast<char>(std::tolower(letter)); });
			if (cliRecorderSignals.contains(command)) {
				sag::match::Signal signal = cliRecorderSignals.at(command);
				std::ranges::for_each(
					recorder_threads, [&](auto& recorder_thread) { recorder_thread.queue().emplace(signal); });
				if (signal == sag::match::Signal::Quit)
					break;
			} else {
				logger->warn("unknown command: {0}", command);
			}
		}

		logger->info("app end");
		logger->flush();
		return 0;
	} catch (std::exception const& exc) {
		logger->error("exception: {0}", exc.what());
		return 1;
	}
}

// NOLINTBEGIN(*magic-numbers)
auto App::create_example_config() -> config::Recorder {
	return config::Recorder{
		.db_file_path = "recorder-db.sqlite",
		.parallel_games = 4,
		.players =
			std::vector<config::Player>{
				{.name = "player-1", .mcts = {.explore_constant = 0.5, .sample_uniformly = true, .simulations = 1000}},
				{.name = "player-2", .mcts = {.explore_constant = 1.5, .sample_uniformly = false, .simulations = 1500}}},
		.log = {.console = config::SimpleLog{}, .file = std::nullopt},
	};
}
// NOLINTEND(*magic-numbers)

}  // namespace app
