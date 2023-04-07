﻿#include <spdlog/common.h>
#include <spdlog/spdlog.h>

#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

#include "sag/rec/MatchConcepts.h"
#include "sag/rec/MatchRecorder.h"
#include "tools/MutexQueue.h"
#include "tools/ThreadHandle.h"

namespace sag::rec {
template <MatchRecorderTypes Types>
struct RecorderThreadHandle : tools::SingleQueuedThreadHandle<sag::rec::Signal> {
 public:
	explicit RecorderThreadHandle(MatchRecorder<Types>&& recorder)
			: tools::SingleQueuedThreadHandle<sag::rec::Signal>(recorder) {}
};

}  // namespace sag::rec

namespace {
auto cli_thread_loop(std::stop_token const& token, tools::MutexQueue<std::string>* queue) -> void {
	std::shared_ptr<spdlog::logger> const logger = spdlog::default_logger();  // todo: make logger configurable/injectable
	logger->info("thread loop starts");
	while (!token.stop_requested()) {
		std::string input;
		logger->debug("waiting for user input");
		std::getline(std::cin, input);
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
}  // namespace

auto main() -> int {
	auto logger = spdlog::default_logger();

	try {
		const std::map<std::string, sag::rec::Signal> cliRecorderSignals = {
			{"record", sag::rec::Signal::Record},
			{"r", sag::rec::Signal::Record},
			{"halt", sag::rec::Signal::Halt},
			{"h", sag::rec::Signal::Halt},
			{"quit", sag::rec::Signal::Quit},
			{"q", sag::rec::Signal::Quit},
			{"exit", sag::rec::Signal::Quit},
			{"status", sag::rec::Signal::Status},
			{"s", sag::rec::Signal::Status},
			{"info", sag::rec::Signal::Status},
		};

		logger->set_level(spdlog::level::info);
		logger->info("recorder start");
		tools::SingleQueuedThreadHandle<std::string> cli_thread(&::cli_thread_loop);

		using TTTRec = sag::rec::TicTacToeRecorder;
		std::vector<sag::rec::RecorderThreadHandle<TTTRec>> recorder_threads;
		recorder_threads.reserve(4);
		for (int i = 0; i < 4; ++i) {
			recorder_threads.emplace_back(sag::rec::RecorderThreadHandle<TTTRec>{{{{}, {}}, {}, {}, {}}});
		}

		while (true) {
			std::string command = cli_thread.queue().wait_and_dequeue();
			std::ranges::transform(
				command, command.begin(), [](unsigned char letter) { return static_cast<char>(std::tolower(letter)); });
			if (cliRecorderSignals.contains(command)) {
				sag::rec::Signal signal = cliRecorderSignals.at(command);
				std::ranges::for_each(
					recorder_threads, [&](auto& recorder_thread) { recorder_thread.queue().emplace(signal); });
				if (signal == sag::rec::Signal::Quit)
					break;
			} else {
				logger->warn("unknown command: {0}", command);
			}
		}

		logger->info("recorder end");
		return 0;
	} catch (std::exception const& exc) {
		logger->error("exception: {0}", exc.what());
		return 1;
	}
}
