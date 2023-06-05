#include "sag/Santorini.h"

#include <catch2/catch_test_macros.hpp>

using namespace sag::santorini;

namespace {

TEST_CASE("Test ordered Positions", "[sag, santorini]") {
	Position row1_col1{1, 1};
	Position row1_col2{1, 2};
	Position row2_col1{2, 1};

	CHECK(row1_col1 < row1_col2);
	CHECK(row1_col1 < row2_col1);
	CHECK(row1_col2 < row2_col1);
}

TEST_CASE("Santorini test", "[sag, santorini]") {
	constexpr Dimensions dim{.rows = 2, .cols = 2, .player_unit_count = 1};
	Board<dim> board{{BoardState::Closed, BoardState::Closed, BoardState::Closed, BoardState::Closed}};
	State<dim> state{board, {}, {}};
	Board<dim> const decoded_board = state.create_board();
	CHECK(board == decoded_board);

	Rules<dim> rules;
	auto roots = rules.list_roots();
	auto root_count = roots.size();
	auto actions = rules.list_actions(roots[0]);
	auto move = rules.list_edges(roots[0], actions[0]);
	Board<dim> new_board = move[0].state().create_board();

	Container<dim> graph{};
	CHECK(graph.is_terminal_at(graph.roots()[0]) == false);
}

}  // namespace
