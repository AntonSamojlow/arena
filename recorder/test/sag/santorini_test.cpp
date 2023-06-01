#include "sag/Santorini.h"

#include <catch2/catch_test_macros.hpp>

using namespace sag::santorini;

namespace {
TEST_CASE("Santorini test", "[sag, santorini]") {
	constexpr Dimensions dim{.rows = 2, .cols = 2, .player_unit_count = 1};
	Board<dim> board{{BoardState::Closed, BoardState::Closed, BoardState::Closed, BoardState::Closed}};
	State<dim> state{board, {}, {}};
	Board<dim> const decoded_board = state.create_board();
	CHECK(board == decoded_board);
}

}  // namespace
