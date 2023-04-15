#pragma once
#include "Match.h"
#include "sag/TicTacToe.h"
#include "sag/storage/MemoryMatchStorage.h"

namespace sag::rec {

struct TicTacToeTestRecorder {
	using graph = tic_tac_toe::Graph;
	using player = RandomPlayer<tic_tac_toe::Graph>;
	using storage = storage::MemoryMatchStorage<typename graph::state, typename graph::action>;
};

static_assert(MatchRecorderTypes<TicTacToeTestRecorder>);

}  // namespace sag::rec
