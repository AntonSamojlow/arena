#include <graph/GraphConcepts.h>
#include <graph/TicTacToe.h>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <random>

template <typename S, typename A>
void test_roots_nonterminal(graph::StateActionGraph<S, A> auto& graph) {
	for (S root : graph.list_roots()) {
		REQUIRE(false == graph.is_terminal_at(root));
	}
}

template <typename S, typename A>
void descend_once_(graph::StateActionGraph<S, A> auto& graph, S& state_in_out, bool randomized) {
	std::mt19937 rng({});  // NOLINT (cert-*)
	std::uniform_real_distribution<double> unit_distribution(0.0, 1.0);
	double const random_value = randomized ? unit_distribution(rng) : 1.0;

	REQUIRE(graph.count_actions(state_in_out) > 0);
	auto actions = graph.list_actions(state_in_out);
	auto action = actions[static_cast<size_t>(std::ceil(random_value * static_cast<double>(actions.size())) - 1)];
	if (!graph.is_expanded_at(state_in_out, action))
		graph.expand(state_in_out, action);
	REQUIRE(graph.is_expanded_at(state_in_out, action));
	double weight_sum = 0.0;
	for (graph::ActionEdge<S> const edge : graph.list_edges(state_in_out, action)) weight_sum += edge.weight();
	REQUIRE(weight_sum == 1.0);
	state_in_out = graph.follow(state_in_out, action, unit_distribution(rng));
}

template <typename S, typename A>
void test_full_descend(graph::StateActionGraph<S, A> auto& graph, bool randomized, std::vector<S>& visited_states) {
	std::mt19937 rng({});  // NOLINT (cert-*)
	std::uniform_real_distribution<double> unit_distribution(0.0, 1.0);
	double const random_value = randomized ? unit_distribution(rng) : 1.0;

	std::vector<S> roots = graph.list_roots();
	S state = roots[static_cast<size_t>(std::ceil(random_value * static_cast<double>(roots.size())) - 1)];
	visited_states.push_back(state);

	while (!graph.is_terminal_at(state)) {
		descend_once_<S, A>(graph, state, randomized);
		visited_states.push_back(state);
	}
}

template <typename S, typename A>
void test_base_operations(graph::StateActionGraph<S, A> auto& graph) {
	std::vector<S> visited_states = {};
	test_roots_nonterminal<S, A>(graph);
	test_full_descend<S, A>(graph, false, visited_states);
	test_full_descend<S, A>(graph, true, visited_states);
}

TEST_CASE("TicTacToe graph tests", "[grap, tictactoe]") {
	graph::tic_tac_toe::TicTacToeGraph graph;
	test_base_operations<graph::tic_tac_toe::StateId, graph::tic_tac_toe::ActionId>(graph);
}
