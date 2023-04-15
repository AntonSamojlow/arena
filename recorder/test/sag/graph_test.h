#include <spdlog/spdlog.h>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <random>

#include "sag/GraphConcepts.h"
#include "sag/GraphOperations.h"

template <typename S, typename A>
void test_roots_nonterminal(sag::GraphContainer<S, A> auto& graph, sag::RulesEngine<S, A> auto const& rules) {
	for (S root : graph.roots()) {
		REQUIRE(false == graph.is_terminal_at(root));
		float const score = rules.score(root).value();
		REQUIRE_THAT(score, !Catch::Matchers::WithinAbs(1.0, 0.0001));
		REQUIRE_THAT(score, !Catch::Matchers::WithinAbs(-1.0, 0.0001));
	}
}

template <typename T, typename R>
	requires std::is_floating_point_v<R>
auto get_random_element(std::vector<T> vec, R random) -> T {
	auto const size = vec.size();
	REQUIRE(size > 0);
	auto const index = static_cast<size_t>(std::ceil(random * static_cast<R>(size)) - 1);
	return vec.at(index);
}

template <typename S, typename A>
auto descend_once_(
	sag::GraphContainer<S, A> auto& graph, sag::RulesEngine<S, A> auto const& rules, S state, bool randomized) -> S {
	std::mt19937 rng({});  // NOLINT (cert-*)
	std::uniform_real_distribution<float> unit_distribution(0.0F, 1.0F);
	float const random_value = randomized ? unit_distribution(rng) : 1.0F;

	auto actions = graph.actions_at(state);
	auto action = get_random_element(actions, random_value);

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
	return sag::follow(edges, tools::UnitValue(unit_distribution(rng)));
}

template <typename S, typename A>
void test_full_descend(sag::GraphContainer<S, A> auto& graph,
	sag::RulesEngine<S, A> auto const& rules,
	bool randomized,
	std::vector<S>& visited_states) {
	std::mt19937 rng({});  // NOLINT (cert-*)
	std::uniform_real_distribution<double> unit_distribution(0.0, 1.0);
	double const random_value = randomized ? unit_distribution(rng) : 1.0;

	auto roots = graph.roots();
	S state = get_random_element(roots, random_value);
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
