#pragma once
// #include <random>
#include <functional>
#include <iterator>

#include "GraphConcepts.h"
#include "GraphOperations.h"
#include "MCTSConcepts.h"

namespace sag::mcts {

template <sag::Identifier S>
struct Path {
	bool terminal = false;
	std::vector<S> values = {};
};

template <typename S, typename A>
	requires sag::Vertices<S, A>
struct Configuration {
	bool sample_actions_uniformly = false;
	std::function<float(S)> initial_value_estimate;
	std::function<float(S, A)> U_bound;
	std::function<UnitValue(void)> random_source;
};

template <typename S, typename A>
auto descend(S state,
	StatsContainer<S> auto& stats,
	sag::GraphContainer<S, A> auto& graph,
	sag::RulesEngine<S, A> auto const& rules,
	Configuration<S, A> config) -> void {
	// ensure start state is listed
	if (!stats.has(state))
		stats..initialize(state, rules.score(state));

	Path const path = select(state, graph, config.sample_actions_uniformly, config.U_bound, config.random_source);
	expand(path, stats, graph, rules, config.initial_value_estimate);
	update(path, stats);
}

// ----------------------------------------------------------------------------
//			the core MCTS routine: selection - expansion - update
// ----------------------------------------------------------------------------

template <typename S, typename A>
	requires sag::Vertices<S, A>
auto select(S state,
	StatsContainer<S> auto& stats,
	sag::GraphContainer<S, A> auto& graph,
	bool sample_actions_uniformly,
	std::function<float(S, A)> U_bound,
	std::function<UnitValue(void)> random_source) -> Path<S> {
	Path path{false, {state}};
	while (!graph.is_terminal_at(state)) {
		const std::vector<A> actions = graph.actions_at(state);

		// check U bound requirement
		auto requirement_failed = [&](A action) {
			if (!graph.is_expanded_at(state, action))
				return true;
			return std::any_of(graph.edges_at(state, action), [&](ActionEdge edge) {
				return !stats.has(edge.state());
			});
		};
		if (std::any_of(actions, requirement_failed))
			return path;

		// cache U_bound values to avoid concurrencey issues (other thread updating stats while this one executes
		// std::min_element)
		std::vector<double> U_bound_values;
		std::transform(actions, std::back_inserter(U_bound_values), [&](A action) {
			return U_bound(state, action);
		});
		int min_index = static_cast<int>(std::distance(U_bound_values.begin(), std::min_element(U_bound_values)));

		if (sample_actions_uniformly) {
			auto edges = graph.edges_at(state, actions[min_index]);
			state = std::min_element(edges, [&](const ActionEdge& left, const ActionEdge& right) {
				return stats.at(left.state()).N < stats.at(right.state()).N;
			})->state();
		} else {
			state = sag::follow(actions[min_index], random_source());
		}
		path.push_back(state);
	}

	path.terminal = true;
	return path;
}

template <typename S, typename A>
	requires sag::Vertices<S, A>
void expand(Path<S> const& path,
	StatsContainer<S> auto& stats,
	sag::GraphContainer<S, A> auto& graph,
	sag::RulesEngine<S, A> auto const& rules,
	std::function<float(S)> initial_value_estimate) {
	S end_state = path.values.back();

	if (path.terminal) {
		if (!stats.has(end_state))
			stats.initialize(end_state, rules.score(end_state));
		return;
	}

	// if path is not terminal, then the U-bound requirement failed
	for (A action : graph.actions_at(end_state)) {
		if (!graph.is_expanded_at(end_state, action))
			sag::expand(graph, rules, end_state, action);

		for (ActionEdge edge : graph.edges_at(end_state, action)) {
			// init the stats entry only if needed - remember the state might have been initialized by another parent!
			if (!stats.has(edge.state()))
				stats.initialize(edge.state(), initial_value_estimate(edge.state()));
		}
	}
}

template <sag::Identifier S>
auto update(Path<S> const& path, StatsContainer<S> auto& stats) -> void {
	if (path.values.empty())
		return;

	float const end_value = stats.at(path.values.back()).Q;
	float sign = static_cast<float>(2 * (path.values.size() % 2) - 1);
	for (auto it = path.values.begin(); it < path.values.end() - 1; ++it, sign *= -1)
		stats.add_visit_result(*it, Score(sign * end_value));

	stats.add_visit(path.values.back());  // basically, we increment N
}

}  // namespace sag::mcts
