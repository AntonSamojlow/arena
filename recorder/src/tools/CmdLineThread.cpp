#include "CmdLineThread.h"

#include <spdlog/spdlog.h>

#include <iostream>
#include <stop_token>
#include <string>

namespace tools {

namespace {
auto thread_loop(std::stop_token const& token, MutexQueue<std::string>& queue) -> void {
	std::shared_ptr<spdlog::logger> logger = spdlog::default_logger();  // todo: make logger configurable/injectable
	logger->info("thread loop starts");
	while (!token.stop_requested()) {
		std::string input;
		logger->debug("waiting for user input");
		std::getline(std::cin, input);
		queue.emplace(input);

		// convert to lower case before comparing to exit conditions
		std::ranges::transform(input, input.begin(), [](unsigned char letter) { return std::tolower(letter); });
		if (input == "q" || input == "quit" || input == "exit") {
			logger->info("exit requested (input: {0})", input);
			break;
		}
	}
	logger->info("thread loop ends");
}
}  // namespace

CmdLineThread::CmdLineThread() : thread_(thread_loop, ref(queue_)) {}

}  // namespace tools
