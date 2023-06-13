#include "app.h"

#include <spdlog/common.h>
#include <tools/MutexQueue.h>

#include <memory>
#include <ranges>
#include <string>
#include <thread>
#include <vector>

#include "app/config.h"
#include "sag/TicTacToe.h"
#include "sag/match/MatchRecorder.h"
#include "sag/mcts/MCTS.h"
#include "sag/mcts/MCTSPlayer.h"
#include "sag/storage/MemoryMatchStorage.h"
#include "tools/BoundedValue.h"

namespace {
struct ReadCommandLoop {
	std::istream& command_source;  // NOLINT(*const-or-ref-data-members)

	auto operator()(std::stop_token const& token, tools::MutexQueue<std::string>* queue) -> void {
		std::shared_ptr<spdlog::logger> const logger =
			spdlog::default_logger();  // todo: make logger configurable/injectable
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

auto App::run(config::Recorder const& config) -> int {
	auto logger = spdlog::default_logger();
	using enum sag::match::Signal;
	try {
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

		logger->set_level(spdlog::level::info);
		logger->info("app start");

		tools::SingleQueuedThreadHandle<std::string> cli_thread(ReadCommandLoop{input_source_});

		using TGraph = sag::tic_tac_toe::Graph;
		using TStorage = sag::storage::MemoryMatchStorage<typename TGraph::state, typename TGraph::action>;
		using TRec = sag::match::MatchRecorder<TGraph, TStorage>;

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

			TRec recorder{std::move(players), {}, {}, {}};
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

}  // namespace app
