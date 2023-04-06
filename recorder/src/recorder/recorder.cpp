#include <spdlog/spdlog.h>
#include <sqlite3.h>

#include <cstddef>
#include <string>
#include <thread>
#include <vector>

#include "recorder/CmdLineThread.h"

auto main() -> int {
	auto logger = spdlog::default_logger();
	logger->info("recorder start");
	recorder::CmdLineThread cmd_line_thread;
	// while (cmd_line_thread.queue().wait_and_dequeue().type != recorder::CmdLineRequest::Type::Quit) {}

	logger->info("recorder end");
	return 0;
}
