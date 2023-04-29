#include "app/app.h"

#include <catch2/catch_test_macros.hpp>

#include "../helpers.h"
#include "app/config.h"

using namespace app;

using namespace std::chrono_literals;

TEST_CASE("AppStopTest", "[app]") {
	std::stringstream input{"h\ns\nr\ns\nq\n"};
	App test_app{input};
	test::TempFilePath const file_path = test::unique_file_path(false);

	const config::Recorder recorder_config = {.db_file_path = file_path.get(),
		.players = {{.name = "player-1", .mcts = {}}, {.name = "player-2", .mcts = {}}},
		.parallel_games = 5};
	const int result = test_app.run(recorder_config);
	CHECK(result == 0);
}
