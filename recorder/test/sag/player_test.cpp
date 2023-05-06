#include <catch2/catch_test_macros.hpp>

#include "sag/ExampleGraph.h"
#include "sag/TicTacToe.h"
#include "sag/match/RandomPlayer.h"
#include "sag/mcts/MCTSPlayer.h"

namespace {

TEST_CASE("Random player test", "[sag, match]") {
	using namespace sag::example;
	sag::match::RandomPlayer<Graph> player_one{};

	SECTION("check equality of default constructed") {
		CHECK(sag::match::RandomPlayer<Graph>{} == sag::match::RandomPlayer<Graph>{});
		CHECK((sag::match::RandomPlayer<Graph>{} != sag::match::RandomPlayer<Graph>{}) == false);
	}

	SECTION("check equality of copied value") {
		sag::match::RandomPlayer<Graph> copy = player_one;
		CHECK(player_one == copy);
		CHECK((player_one != copy) == false);
	}

	SECTION("check random play") {
		std::vector<ActionEdges> const TERMINAL = {};
		GraphStructure const graph_structure = {
			{1,
				{
					{{1.0, 2}},
					{{1.0, 3}},
				}},
			{2, TERMINAL},
			{3, {{{1.0, 4}}}},
			{4, {TERMINAL}},
		};
		Graph::rules const rules{graph_structure};
		Graph::container graph{rules};

		std::vector<Graph::action> plays;
		plays.reserve(100);
		for (size_t i = 0; i < 100; ++i) {
			plays.push_back(player_one.choose_play(graph.roots()[0], graph, rules));
		}

		// verify that random play chooses between both available actions
		CHECK(std::ranges::find(plays, 0) != plays.end());
		CHECK(std::ranges::find(plays, 1) != plays.end());
	}
}

TEST_CASE("MCTS player test", "[sag, mcts]") {
	using namespace sag::tic_tac_toe;

	sag::mcts::MCTSPlayer<Graph> const mcts_one(1000);

	SECTION("check equality oF default constructed") {
		CHECK(sag::mcts::MCTSPlayer<Graph>{} == sag::mcts::MCTSPlayer<Graph>{});
		CHECK((sag::mcts::MCTSPlayer<Graph>{} != sag::mcts::MCTSPlayer<Graph>{}) == false);
	}

	SECTION("check equality of copied value") {
		sag::mcts::MCTSPlayer<Graph> copy = mcts_one;
		CHECK(mcts_one == copy);
		CHECK((mcts_one != copy) == false);
	}
}
}  // namespace
