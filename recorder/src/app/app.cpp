#include "app.h"

#include <spdlog/common.h>
#include <tools/MutexQueue.h>

#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "config.h"
#include "sag/match/MatchRecorder.h"
#include "sag/match/TicTacToeTestRecorder.h"

namespace {
struct ReadCommandLoop {
	std::istream& command_source;

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

auto App::run() -> int {
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

		ReadCommandLoop command_loop = {.command_source = input_source_};
		tools::SingleQueuedThreadHandle<std::string> cli_thread(command_loop);

		using TestRec = sag::match::TicTacToeTestRecorder;
		std::vector<sag::match::RecorderThreadHandle<TestRec>> recorder_threads;
		recorder_threads.reserve(4);
		for (int i = 0; i < 4; ++i) {
			recorder_threads.emplace_back(sag::match::RecorderThreadHandle<TestRec>{{{{}, {}}, {}, {}, {}}});
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
