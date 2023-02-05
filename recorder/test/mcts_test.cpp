#include "sag/mcts/MCTS.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "sag/ExampleGraph.h"
#include "sag/mcts/MCTSConcepts.h"
#include "sag/mcts/StatsContainer.h"

using namespace sag::example;

TEST_CASE("MCTS test", "[mcts]") {
	std::vector<sag::example::ActionEdges> const TERMINAL = {};
	sag::example::GraphStructure const graph_structure = {
		{1,
			{
				{{1.0F / 3, 2}, {2.0F / 3, 3}},
				{{0.25F, 5}, {0.75F, 6}},
			}},
		{2, TERMINAL},
		{3, {{{1.0, 4}}}},
		{4, TERMINAL},
		{5, TERMINAL},
		{6, {{{0.5, 7}, {0.5, 8}}}},
		{7, TERMINAL},
		{8, {{{1.0, 9}}}},
		{9, TERMINAL},
	};

	ExampleRulesEngine const rules(graph_structure);
	ExampleGraph graph(rules);
	sag::mcts::Statistics<State> stats;
	sag::mcts::BaseMCTS<State, Action> mcts_algo(true, 2);
	State const root = graph.roots()[0];

	// Test that MCTS finds the correct (true) action values at root 1 are: 1/3 and -1/4
	CHECK(stats.size() == 0);
	for (size_t i = 0; i < 10'000; i++) {
		mcts_algo.descend(root, stats, graph, rules);
	}
	CHECK(stats.size() > 0);
	auto result = sag::mcts::action_estimates_at<State, Action>(root, graph, stats);
	CHECK(result.size() == 2);
	CHECK_THAT(result[0], Catch::Matchers::WithinAbs(1.0 / 3, 0.001));
	// CHECK_THAT(result[1], Catch::Matchers::WithinAbs(-0.25, 0.001));
}
