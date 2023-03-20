#include <spdlog/spdlog.h>
#include <sqlite3.h>

#include <cstddef>
#include <string>

#include "sag/TicTacToe.h"
#include "sag/rec/MatchRecorder.h"
#include "sag/rec/Player.h"
#include "sag/storage/MemoryMatchStorage.h"

struct ExampleTypes {
	using graph = sag::tic_tac_toe::Graph;
	using player = sag::rec::RandomPlayer<graph>;
	using storage = sag::storage::MemoryMatchStorage<typename graph::state, typename graph::action>;
};

auto main() -> int {
	auto logger = spdlog::default_logger();
	logger->info("recorder start");

	// sag::rec::MatchRecorder<ExampleTypes> recorder;

	logger->info("recorder end");
	return 0;
}
