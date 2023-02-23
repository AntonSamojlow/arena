#include <spdlog/spdlog.h>
#include <sqlite3.h>

#include <cstddef>
#include <string>

auto main() -> int {
	auto logger = spdlog::default_logger();
	logger->info("recorder start");
	logger->info("recorder end");
	return 0;
}
