#include <spdlog/spdlog.h>

auto main() -> int {
	auto logger = spdlog::default_logger();
	logger->info("recorder start");
	logger->info("recorder end");
	return 0;
}
