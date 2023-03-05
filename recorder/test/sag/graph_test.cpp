#include "graph_test.h"

#include "sag/ExampleGraph.h"
#include "sag/TicTacToe.h"

TEST_CASE("TicTacToe graph tests", "[sag]") {
	sag::tic_tac_toe::Graph::container graph;
	sag::tic_tac_toe::Graph::rules const rules;
	test_base_operations<sag::tic_tac_toe::Graph::state, sag::tic_tac_toe::Graph::action>(graph, rules);

	// test output
	auto logger = spdlog::default_logger();
	logger->info("root: {}", graph.to_string(graph.roots().front()));
	std::vector<sag::tic_tac_toe::Graph::state> visited_states{};
	test_full_descend<sag::tic_tac_toe::Graph::state, sag::tic_tac_toe::Graph::action>(
		graph, rules, false, visited_states);
	logger->info("logging all states of a full descend");
	for (auto state : visited_states)
		logger->info("\n{}", graph.to_string_formatted(state));
}

TEST_CASE("ExampleGraph tests", "[graph]") {
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
	sag::example::Rules const rules(graph_structure);
	sag::example::Container graph(rules);
	test_base_operations<sag::example::Graph::state, sag::example::Graph::action>(graph, rules);
}
