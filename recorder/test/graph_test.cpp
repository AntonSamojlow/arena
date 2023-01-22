#include <sag/ExampleGraph.h>
#include <sag/GraphConcepts.h>
#include <sag/GraphOperations.h>
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
auto descend_once_(
	sag::GraphContainer<S, A> auto& graph, sag::RulesEngine<S, A> auto const& rules, S state, bool randomized) -> S {
	std::mt19937 rng({});  // NOLINT (cert-*)
	std::uniform_real_distribution<float> unit_distribution(0.0F, 1.0F);
	float const random_value = randomized ? unit_distribution(rng) : 1.0F;

	auto actions = graph.actions_at(state);
	size_t const actions_size = actions.size();
	REQUIRE(actions_size > 0);
	REQUIRE(actions_size < static_cast<size_t>(std::numeric_limits<float>::max()));

	auto const index = static_cast<size_t>(std::ceil(random_value * static_cast<float>(actions_size)) - 1);
	auto action = actions[index];

	// test expansion
	sag::expand(graph, rules, state, action);
	REQUIRE(graph.is_expanded_at(state, action));

	// test edges sum to one
	auto edges = graph.edges_at(state, action);
	float weight_sum = 0.0;
	for (sag::ActionEdge<S> const& edge : edges) {
		weight_sum += edge.weight().value();
	}
	REQUIRE(weight_sum == 1.0F);

	// run 'follow' operation
	return sag::follow(edges, UnitValue(unit_distribution(rng)));
}

template <typename S, typename A>
void test_full_descend(sag::GraphContainer<S, A> auto& graph,
	sag::RulesEngine<S, A> auto const& rules,
	bool randomized,
	std::vector<S>& visited_states) {
	std::mt19937 rng({});  // NOLINT (cert-*)
	std::uniform_real_distribution<double> unit_distribution(0.0, 1.0);
	double const random_value = randomized ? unit_distribution(rng) : 1.0;

	std::vector<S> roots = graph.roots();
	size_t const roots_size = roots.size();
	REQUIRE(roots_size > 0);
	REQUIRE(roots_size < static_cast<size_t>(std::numeric_limits<double>::max()));

	auto const index = static_cast<size_t>(std::ceil(random_value * static_cast<double>(roots_size)) - 1);
	S state = roots[index];
	visited_states.push_back(state);

	while (!graph.is_terminal_at(state)) {
		state = descend_once_<S, A>(graph, rules, state, randomized);
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
	test_full_descend<S, A>(graph, rules, false, visited_states);
	test_full_descend<S, A>(graph, rules, true, visited_states);

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
	test_full_descend<sag::tic_tac_toe::StateId, sag::tic_tac_toe::ActionId>(
		graph, graph.rules_engine(), false, visited_states);
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
