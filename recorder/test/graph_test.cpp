#include <sag/ExampleGraph.h>
#include <sag/GraphConcepts.h>
#include <sag/TicTacToe.h>
#include <spdlog/spdlog.h>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <random>

template <typename S, typename A>
void test_roots_nonterminal(sag::GraphContainer<S, A> auto& graph, sag::RulesEngine<S, A> auto const& rules) {
	for (S root : graph.roots()) {
		REQUIRE(false == graph.is_terminal_at(root));
		float const score = rules.score(root).value();
		REQUIRE_THAT(score, !Catch::Matchers::WithinAbs(1.0, 0.0001));
		REQUIRE_THAT(score, !Catch::Matchers::WithinAbs(-1.0, 0.0001));
	}
}

template <typename S, typename A>
void descend_once_(sag::GraphContainer<S, A> auto& graph, S& state_in_out, bool randomized) {
	//std::mt19937 rng({});  // NOLINT (cert-*)
	//std::uniform_real_distribution<double> unit_distribution(0.0, 1.0);
	//double const random_value = randomized ? unit_distribution(rng) : 1.0;

	//auto actions = graph.actions_at(state_in_out);
	//REQUIRE(actions.size() > 0);
	//auto action = actions[static_cast<size_t>(std::ceil(random_value * static_cast<double>(actions.size())) - 1)];

	// if (!graph.is_expanded_at(state_in_out, action))
	//	graph.expand_at(state_in_out, action);
	// REQUIRE(graph.is_expanded_at(state_in_out, action));
	// float weight_sum = 0.0;
	// for (sag::ActionEdge<S> const& edge : graph.edges_at(state_in_out, action)) {
	//	weight_sum += edge.weight().value();
	// }
	// REQUIRE(weight_sum == 1.0);
	// state_in_out = graph.follow(state_in_out, action, unit_distribution(rng));
}

template <typename S, typename A>
void test_full_descend(sag::GraphContainer<S, A> auto& graph, bool randomized, std::vector<S>& visited_states) {
	std::mt19937 rng({});  // NOLINT (cert-*)
	std::uniform_real_distribution<double> unit_distribution(0.0, 1.0);
	double const random_value = randomized ? unit_distribution(rng) : 1.0;

	std::vector<S> roots = graph.roots();
	S state = roots[static_cast<size_t>(std::ceil(random_value * static_cast<double>(roots.size())) - 1)];
	visited_states.push_back(state);

	while (!graph.is_terminal_at(state)) {
		descend_once_<S, A>(graph, state, randomized);
		visited_states.push_back(state);
	}
}

template <typename S, typename A>
void check_terminal_state(sag::RulesEngine<S, A> auto const& rules, S const& state) {
	float const score = rules.score(state).value();
	REQUIRE_THAT(score, Catch::Matchers::WithinAbs(1.0, 0.0001) || Catch::Matchers::WithinAbs(-1.0, 0.0001));
	REQUIRE(rules.list_actions(state).empty());
}

template <typename S, typename A>
void test_base_operations(sag::GraphContainer<S, A> auto& graph, sag::RulesEngine<S, A> auto const& rules) {
	std::vector<S> visited_states = {};
	test_roots_nonterminal<S, A>(graph, rules);
	test_full_descend<S, A>(graph, false, visited_states);
	test_full_descend<S, A>(graph, true, visited_states);

	std::vector<S> terminal_states = {};
	for (S state : visited_states) {
		if (graph.is_terminal_at(state)) {
			terminal_states.push_back(state);
			check_terminal_state<S, A>(rules, state);
		}
	}
	REQUIRE(terminal_states.size() == 2);
}

// Test various graphs

TEST_CASE("TicTacToe graph tests", "[graph, tictactoe]") {
	sag::tic_tac_toe::TicTacToeGraph graph;
	test_base_operations<sag::tic_tac_toe::StateId, sag::tic_tac_toe::ActionId>(graph, graph.rules_engine());

	// test output
	auto logger = spdlog::default_logger();
	logger->info("root: {}", graph.stringify(graph.roots().front()));
	std::vector<sag::tic_tac_toe::StateId> visited_states{};
	test_full_descend<sag::tic_tac_toe::StateId, sag::tic_tac_toe::ActionId>(graph, false, visited_states);
	logger->info("logging all states of a full descend");
	for (auto state : visited_states)
		logger->info("\n{}", graph.stringify_formatted(state));
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

	sag::example::ExampleRulesEngine const rules(graph_structure);
	sag::example::ExampleGraph graph(rules);
	test_base_operations<sag::example::State, sag::example::Action>(graph, graph.rules_engine());
}
