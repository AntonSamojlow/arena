#include "cli.h"

#include <spdlog/spdlog.h>

#include <iostream>
#include <algorithm>

namespace app {
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
}  // namespace app
