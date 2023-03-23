#include "sag/rec/Recorder.h"

#include <spdlog/spdlog.h>
#include <sqlite3.h>

#include <cstddef>
#include <string>
#include <thread>
#include <vector>

#include "sag/TicTacToe.h"
#include "sag/rec/MatchRecorder.h"
#include "sag/rec/Player.h"
#include "sag/storage/MemoryMatchStorage.h"

using namespace sag::tic_tac_toe;
using namespace std::literals;

auto main() -> int {
	auto logger = spdlog::default_logger();
	logger->info("recorder start");

	sag::rec::Recorder recorder(sag::rec::MatchRecorder<sag::rec::TicTacToeRecorder> {});

	logger->info("recorder end");
	return 0;
}
