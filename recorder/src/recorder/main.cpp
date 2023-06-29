#include "app/app.h"
#include "app/config.h"

using namespace app;

auto main() -> int {
	App app{};
	const auto read_config = config::read<config::Recorder>("C:/Repos/arena/config.test.json");
	if (read_config) {
		return app.run(read_config.value());
	}
	return read_config.error().code;
}
