#pragma once
// #include <random>
#include <algorithm>
#include <functional>
#include <iterator>
#include <random>

#include "StatsContainer.h"
#include "sag/ExampleGraph.h"
#include "sag/GraphConcepts.h"
#include "sag/GraphOperations.h"

namespace sag::mcts {
template <sag::Identifier S>
struct Path {
	bool terminal = false;
	std::vector<S> values = {};
};

// --------------------------------------------------------------------------------------------------------------------
//			toolset: action estimate
// --------------------------------------------------------------------------------------------------------------------

/// compute stats for each action: average over its resulting state values
template <Graph G>
auto action_estimates_at(
	typename G::state state, typename G::container const& graph, StatsContainer<typename G::state> auto const& stats)
	-> std::vector<float> {
	std::vector<float> action_estimates;
	for (typename G::action action : graph.actions_at(state)) {
		float action_value = 0.0;
		for (auto edge : graph.edges_at(state, action))
			action_value += edge.weight().value() * stats.at(edge.state()).Q;
		action_estimates.push_back(action_value);
	}
	return action_estimates;
}

// --------------------------------------------------------------------------------------------------------------------
//			the core MCTS routine: selection - expansion - update
// --------------------------------------------------------------------------------------------------------------------

template <Graph G>
auto mcts_run(typename G::state state,
	StatsContainer<typename G::state> auto& stats,
	typename G::container& graph,
	typename G::rules const& rules,
	bool sample_actions_uniformly,
	std::function<float(typename G::state, typename G::action)> upper_confidence_bound,
	std::function<tools::UnitValue(void)> random_source,
	std::function<tools::Score(typename G::state)> initial_value_estimate) -> Path<typename G::state> {
	// ensure start state is listed
	if (!stats.has(state))
		stats.initialize(state, rules.score(state));
	// run one standard mcts pass
	auto const path = select<G>(state, stats, graph, sample_actions_uniformly, upper_confidence_bound, random_source);
	expand<G>(path, stats, graph, rules, initial_value_estimate);
	update<typename G::state>(path, stats);
	return path;
}

template <Graph G>
auto select(typename G::state state,
	StatsContainer<typename G::state> auto& stats,
	typename G::container& graph,
	bool sample_actions_uniformly,
	std::function<float(typename G::state, typename G::action)> upper_confidence_bound,
	std::function<tools::UnitValue(void)>& random_source) -> Path<typename G::state> {
	Path<typename G::state> path{false, {state}};
	while (!graph.is_terminal_at(state)) {
		auto const actions = graph.actions_at(state);

		// check upper_confidence_bound requirement
		auto requirement_failed = [&](typename G::action action) {
			if (!graph.is_expanded_at(state, action))
				return true;
			return std::ranges::any_of(
				graph.edges_at(state, action), [&](ActionEdge<typename G::state> edge) { return !stats.has(edge.state()); });
		};
		if (std::ranges::any_of(actions, requirement_failed))
			return path;

		// cache upper_confidence_bound values to avoid concurrencey issues
		// (another thread might update stats while this one executes std::min_element)
		std::vector<double> ucb_values;
		std::ranges::transform(
			actions, std::back_inserter(ucb_values), [&](auto action) { return upper_confidence_bound(state, action); });
		auto min_index = static_cast<size_t>(std::distance(ucb_values.begin(), std::ranges::min_element(ucb_values)));

		if (sample_actions_uniformly) {
			auto edges = graph.edges_at(state, actions[min_index]);
			state = std::ranges::min_element(
				edges, [&](const ActionEdge<typename G::state>& left, const ActionEdge<typename G::state>& right) {
					return stats.at(left.state()).N < stats.at(right.state()).N;
				})->state();
		} else {
			state = sag::follow<typename G::state>(graph.edges_at(state, actions[min_index]), random_source());
		}
		path.values.push_back(state);
	}

	path.terminal = true;
	return path;
}

template <Graph G>
void expand(Path<typename G::state> const& path,
	StatsContainer<typename G::state> auto& stats,
	typename G::container& graph,
	typename G::rules const& rules,
	std::function<tools::Score(typename G::state)> initial_value_estimate) {
	typename G::state end_state = path.values.back();

	if (path.terminal) {
		if (!stats.has(end_state))
			stats.initialize(end_state, rules.score(end_state));
		return;
	}

	// if path is not terminal, then the U-bound requirement failed
	for (auto action : graph.actions_at(end_state)) {
		if (!graph.is_expanded_at(end_state, action))
			sag::expand<G>(graph, rules, end_state, action);

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
	float sign = 2 * static_cast<float>(path.values.size() % 2) - 1;
	for (auto it = path.values.begin(); it < path.values.end() - 1; ++it, sign *= -1)
		stats.add_visit_result(*it, tools::Score(sign * end_value));

	stats.add_visit(path.values.back());  // basically, we increment N
}

// --------------------------------------------------------------------------------------------------------------------
//			standard ingredients: default upper confidence bound and random rollout
// --------------------------------------------------------------------------------------------------------------------

template <Graph G>
[[nodiscard]] auto upper_confidence_bound(typename G::state state,
	typename G::action action,
	typename G::container& graph,
	StatsContainer<typename G::state> auto& stats,
	tools::NonNegative explore_constant) -> float {
	float value_estimate = 0.0;
	int action_visits = 0;
	for (auto egde : graph.edges_at(state, action)) {
		value_estimate += (egde.weight().value() * stats.at(egde.state()).Q);
		action_visits += stats.at(egde.state()).N;
	}

	return value_estimate -
				 explore_constant.value() * static_cast<float>(std::sqrt(stats.at(state).N / (1 + action_visits)));
}

template <Graph G>
auto random_rollout(typename G::state state,
	typename G::container& graph,
	typename G::rules const& rules,
	std::function<tools::UnitValue(void)>& random_source) -> tools::Score {
	int rollout_length = 0;
	while (!graph.is_terminal_at(state)) {
		// pick a random action, expand and follow
		auto actions = graph.actions_at(state);
		size_t index = std::min(actions.size() - 1,
			static_cast<size_t>(std::floor(static_cast<float>(actions.size()) * random_source().value())));
		typename G::action action = actions[index];
		if (!graph.is_expanded_at(state, action)) {
			sag::expand<G>(graph, rules, state, action);
		}
		state = sag::follow(graph.edges_at(state, action), random_source());
		rollout_length++;
	}
	float value = (1 - 2 * static_cast<float>(rollout_length % 2)) * rules.score(state).value();
	return tools::Score(value);
}

// --------------------------------------------------------------------------------------------------------------------
//			Base MCTS implementation
// --------------------------------------------------------------------------------------------------------------------

template <Graph G>
class BaseMCTS {
 public:
	/// default base mcts (incl seed))
	BaseMCTS() = default;

	explicit BaseMCTS(std::random_device::result_type seed) : rng_(seed) {}

	/// generate a base mcts with a randomized seed
	BaseMCTS(bool sample_actions_uniformly, tools::NonNegative explore_constant)
			: sample_actions_uniformly_(sample_actions_uniformly),
				explore_constant_(explore_constant),
				rng_(std::random_device()()) {}

	auto operator==(const BaseMCTS& other) const -> bool = default;

	auto descend(typename G::state state,
		StatsContainer<typename G::state> auto& stats,
		typename G::container& graph,
		typename G::rules const& rules) -> void {
		std::function<tools::UnitValue(void)> random_source = [this]() -> tools::UnitValue {
			return tools::UnitValue(unit_distribution_(rng_));
		};

		std::function<float(typename G::state, typename G::action)> U_bound_lambda = [&](typename G::state target_state,
																																									 typename G::action action) -> float {
			return upper_confidence_bound<G>(target_state, action, graph, stats, explore_constant_);
		};

		std::function<tools::Score(typename G::state)> initial_value_estimate =
			[&](typename G::state start_state) -> tools::Score {
			return random_rollout<G>(start_state, graph, rules, random_source);
		};

		mcts_run<G>(
			state, stats, graph, rules, sample_actions_uniformly_, U_bound_lambda, random_source, initial_value_estimate);
	}

 private:
	bool sample_actions_uniformly_ = false;
	tools::NonNegative explore_constant_{1.0F};
	std::mt19937 rng_;
	std::uniform_real_distribution<float> unit_distribution_ = std::uniform_real_distribution<float>(0.0, 1.0);
};

static_assert(std::regular<BaseMCTS<sag::example::Graph>>);

}  // namespace sag::mcts
