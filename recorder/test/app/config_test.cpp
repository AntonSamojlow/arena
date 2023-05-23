#include "app/config.h"

#include <catch2/catch_test_macros.hpp>
#include <filesystem>

#include "../helpers.h"

using namespace app::config;

using namespace std::chrono_literals;

TEST_CASE("ConfigReadWriteTest", "[app]") {
	MCTS const mcts_1 = {
		.explore_constant = 0.5,
		.sample_uniformly = true,
		.simulations = 10,
	};
	MCTS const mcts_2 = {.explore_constant = 1.5, .sample_uniformly = false, .simulations = 12};

	Recorder const recorder_config = {.db_file_path = "some_db_file",
		.parallel_games = 5,
		.players = {{.name = "player-1", .mcts = mcts_1}, {.name = "player-2", .mcts = mcts_2}}};

	test::TempFilePath const file_path = test::unique_file_path(false);
	CHECK(write(recorder_config, file_path.get()).has_value());
	CHECK(std::filesystem::file_size(file_path.get()) != 0);

	const auto read_config = read<Recorder>(file_path.get());
	CHECK(read_config.has_value());
	CHECK(read_config.value() == recorder_config);
}
