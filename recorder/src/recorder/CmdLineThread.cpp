#include "CmdLineThread.h"

#include <spdlog/spdlog.h>

#include <iostream>
#include <stop_token>
#include <string>

namespace recorder {

namespace {
auto thread_loop(std::stop_token const& token, tools::MutexQueue<CmdLineRequest>& queue) -> void {
	std::shared_ptr<spdlog::logger> logger = spdlog::default_logger();  // todo: make logger configurable/injectable
	logger->info("thread loop starts");
	while (!token.stop_requested()) {
			std::string input;
			logger->debug("waiting for user input");
			std::getline(std::cin, input);

			if (input == "q" || input == "quit") {
				logger->info("quit requested");
				queue.emplace(CmdLineRequest{.type = CmdLineRequest::Type::Quit});
				break;
			}

			logger->warn("unknown command: {}", input);
	}
	logger->info("thread loop ends");
}
}  // namespace

CmdLineThread::CmdLineThread() : thread_(thread_loop, ref(queue_)) {}

}  // namespace recorder
