#include <vcruntime.h>

#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <vector>

#include "sag/ExampleGraph.h"
#include "sag/TicTacToe.h"
#include "sag/match/RandomPlayer.h"
#include "sag/mcts/MCTSPlayer.h"

namespace {

TEST_CASE("Random player test", "[sag, match]") {
	using namespace sag::example;
	sag::match::RandomPlayer<Graph> player{};

	SECTION("check equality of default constructed") {
		CHECK(sag::match::RandomPlayer<Graph>{} == sag::match::RandomPlayer<Graph>{});
		CHECK((sag::match::RandomPlayer<Graph>{} != sag::match::RandomPlayer<Graph>{}) == false);
	}

	SECTION("check equality of copied value") {
		sag::match::RandomPlayer<Graph> copy = player;
		CHECK(player == copy);
		CHECK((player != copy) == false);
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
			plays.push_back(player.choose_play(graph.roots()[0], graph, rules));
		}

		// verify that random play chooses between both available actions
		CHECK(std::ranges::find(plays, 0) != plays.end());
		CHECK(std::ranges::find(plays, 1) != plays.end());
	}
}

TEST_CASE("MCTS player test", "[sag, mcts]") {
	using namespace sag::tic_tac_toe;

	sag::mcts::MCTSPlayer<Graph> deterministic_player(1'000);
	sag::mcts::MCTSPlayer<Graph> probabilisitc_player(1'000, tools::NonNegative{0.0001F});

	SECTION("check equality oF default constructed") {
		CHECK(sag::mcts::MCTSPlayer<Graph>{} == sag::mcts::MCTSPlayer<Graph>{});
		CHECK((sag::mcts::MCTSPlayer<Graph>{} != sag::mcts::MCTSPlayer<Graph>{}) == false);
		CHECK(sag::mcts::MCTSPlayer<Graph>{} != probabilisitc_player);
	}

	SECTION("check equality of copied value") {
		sag::mcts::MCTSPlayer<Graph> copy = deterministic_player;
		CHECK(deterministic_player == copy);
		CHECK((deterministic_player != copy) == false);
		CHECK(deterministic_player != probabilisitc_player);
	}

	SECTION("compare deterministic and probabilistic") {
		Graph::container graph{};
		Graph::rules const rules{};
		Board const win_next_turn = {0, 2, 2, 0, 0, 0, 1, 1, 0};
		Graph::action const optimal_play = 8;
		Graph::state const state = rules.encode(win_next_turn);

		CHECK(add_state<Graph>(graph, rules, state));

		auto deterministic_play = deterministic_player.choose_play(state, graph, rules);
		CHECK(deterministic_play == optimal_play);

		size_t const sample_size = 10;  // increase (or adjust estimates_exponent_ for probabilisitc_player) if test is unstable
		std::vector<Graph::action> probabilistic_plays;
		probabilistic_plays.reserve(sample_size);
		for (size_t i = 0; i < sample_size; ++i) {
			probabilistic_plays.emplace_back(probabilisitc_player.choose_play(state, graph, rules));
		}

		CHECK(std::any_of(probabilistic_plays.cbegin(), probabilistic_plays.cend(), [](Graph::action const action) {
			return action != optimal_play;
		}));
	}
}
}  // namespace
