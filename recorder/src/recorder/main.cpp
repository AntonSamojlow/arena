#include <spdlog/spdlog.h>

#include <exception>
#include <filesystem>

#include "app/app.h"
#include "app/config.h"
#include "tools/Failure.h"

using namespace app;

auto main(int argc, char* argv[]) -> int {
	auto logger = spdlog::default_logger();
	auto log_error = [&logger](tools::Failure const& fail) {
		logger->error("Failure ({}): '{}'", fail.code, fail.reason);
		return fail.code;
	};

	try {
		if (argc != 2) {
			logger->info("Usage: recorder <PATH-TO-CONFIG>");
			logger->info("(This wil either read the config if it exists or generate an example config file.)");
			return 0;
		}

		std::filesystem::path const path = {argv[1]};  // NOLINT(*pointer-arithmetic)
		if (!std::filesystem::exists(path)) {
			auto const write_config = config::write<config::Recorder>(App::create_example_config(), path);
			return write_config.has_value() ? 0 : log_error(write_config.error());
		}

		if (!std::filesystem::is_regular_file(path) && !std::filesystem::is_symlink(path)) {
			logger->error("Provided path '{}' is neither file nor a symlink", path.string());
			return 1;
		}

		App app{};
		auto const read_config = config::read<config::Recorder>(path);
		return read_config.has_value() ? app.run(read_config.value()) : log_error(read_config.error());

	} catch (std::exception const& exc) {
		logger->error("Unhandled exception: {}", exc.what());
	}
}
