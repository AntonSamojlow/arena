#pragma once
// #include <random>
#include <algorithm>
#include <functional>
#include <iterator>
#include <random>

#include "MCTSConcepts.h"
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
template <typename S, typename A>
std::vector<float> action_estimates_at(
	S state, sag::GraphContainer<S, A> auto const& graph, StatsContainer<S> auto const& stats) {
	std::vector<float> action_estimates;
	for (A action : graph.actions_at(state)) {
		float action_value = 0.0;
		for (sag::ActionEdge<S> edge : graph.edges_at(state, action))
			action_value += edge.weight().value() * stats.at(edge.state()).Q;
		action_estimates.push_back(action_value);
	}
	return action_estimates;
}

// --------------------------------------------------------------------------------------------------------------------
//			the core MCTS routine: selection - expansion - update
// --------------------------------------------------------------------------------------------------------------------

template <typename S, typename A>
	requires sag::Vertices<S, A>
auto mcts_run(S state,
	StatsContainer<S> auto& stats,
	sag::GraphContainer<S, A> auto& graph,
	sag::RulesEngine<S, A> auto const& rules,
	bool sample_actions_uniformly,
	std::function<float(S, A)> upper_confidence_bound,
	std::function<UnitValue(void)> random_source,
	std::function<Score(S)> initial_value_estimate) -> Path<S> {
	// ensure start state is listed
	if (!stats.has(state))
		stats.initialize(state, rules.score(state));
	// run one standard mcts pass
	Path<S> const path =
		select<S, A>(state, stats, graph, sample_actions_uniformly, upper_confidence_bound, random_source);
	expand<S, A>(path, stats, graph, rules, initial_value_estimate);
	update<S>(path, stats);
	return path;
}

template <typename S, typename A>
	requires sag::Vertices<S, A>
auto select(S state,
	StatsContainer<S> auto& stats,
	sag::GraphContainer<S, A> auto& graph,
	bool sample_actions_uniformly,
	std::function<float(S, A)> upper_confidence_bound,
	std::function<UnitValue(void)> random_source) -> Path<S> {
	Path<S> path{false, {state}};
	while (!graph.is_terminal_at(state)) {
		const std::vector<A> actions = graph.actions_at(state);

		// check upper_confidence_bound requirement
		auto requirement_failed = [&](A action) {
			if (!graph.is_expanded_at(state, action))
				return true;
			return std::ranges::any_of(graph.edges_at(state, action), [&](ActionEdge<S> edge) {
				return !stats.has(edge.state());
			});
		};
		if (std::ranges::any_of(actions, requirement_failed))
			return path;

		// cache upper_confidence_bound values to avoid concurrencey issues
		// (another thread might update stats while this one executes std::min_element)
		std::vector<double> ucb_values;
		std::ranges::transform(actions, std::back_inserter(ucb_values), [&](A action) {
			return upper_confidence_bound(state, action);
		});
		size_t min_index = static_cast<size_t>(std::distance(ucb_values.begin(), std::ranges::min_element(ucb_values)));

		if (sample_actions_uniformly) {
			auto edges = graph.edges_at(state, actions[min_index]);
			state = std::ranges::min_element(edges, [&](const ActionEdge<S>& left, const ActionEdge<S>& right) {
				return stats.at(left.state()).N < stats.at(right.state()).N;
			})->state();
		} else {
			state = sag::follow<S>(graph.edges_at(state, actions[min_index]), random_source());
		}
		path.values.push_back(state);
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
	std::function<Score(S)> initial_value_estimate) {
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
	float sign = 2 * static_cast<float>(path.values.size() % 2) - 1;
	for (auto it = path.values.begin(); it < path.values.end() - 1; ++it, sign *= -1)
		stats.add_visit_result(*it, Score(sign * end_value));

	stats.add_visit(path.values.back());  // basically, we increment N
}

// --------------------------------------------------------------------------------------------------------------------
//			standard ingredients: default upper confidence bound and random rollout
// --------------------------------------------------------------------------------------------------------------------

template <typename S, typename A>
	requires sag::Vertices<S, A>
[[nodiscard]] auto upper_confidence_bound(
	S state, A action, sag::GraphContainer<S, A> auto& graph, StatsContainer<S> auto& stats, NonNegative explore_constant)
	-> float {
	float value_estimate = 0.0;
	int action_visits = 0;
	for (ActionEdge egde : graph.edges_at(state, action)) {
		value_estimate += (egde.weight().value() * stats.at(egde.state()).Q);
		action_visits += stats.at(egde.state()).N;
	}

	return value_estimate -
				 explore_constant.value() * static_cast<float>(std::sqrt(stats.at(state).N / (1 + action_visits)));
}

template <typename S, typename A>
	requires sag::Vertices<S, A>
auto random_rollout(S state,
	sag::GraphContainer<S, A> auto& graph,
	sag::RulesEngine<S, A> auto const& rules,
	std::function<UnitValue(void)> random_source) -> Score {
	int rollout_length = 0;
	while (!graph.is_terminal_at(state)) {
		// pick a random action, expand and follow
		std::vector<A> actions = graph.actions_at(state);
		size_t index = static_cast<size_t>(std::floor(static_cast<float>(actions.size()) * random_source().value()));
		A action = actions[index];
		if (!graph.is_expanded_at(state, action)) {
			sag::expand(graph, rules, state, action);
		}
		state = sag::follow(graph.edges_at(state, action), random_source());
		rollout_length++;
	}
	float value = (1 - 2 * static_cast<float>(rollout_length % 2)) * rules.score(state).value();
	return Score(std::move(value));
}

// --------------------------------------------------------------------------------------------------------------------
//			Base MCTS implementation
// --------------------------------------------------------------------------------------------------------------------

template <typename S, typename A>
	requires sag::Vertices<S, A>
class BaseMCTS {
 public:
	BaseMCTS() {
		std::random_device rd;
		rng_ = std::mt19937(rd());
	}

	BaseMCTS(bool sample_actions_uniformly, NonNegative explore_constant) : BaseMCTS() {
		sample_actions_uniformly_ = sample_actions_uniformly;
		explore_constant_ = explore_constant;
	}

	auto descend(S state,
		StatsContainer<S> auto& stats,
		sag::GraphContainer<S, A> auto& graph,
		sag::RulesEngine<S, A> auto const& rules) -> void {
		std::function<UnitValue(void)> random_source = [this]() -> UnitValue {
			return UnitValue(std::move(unit_distribution_(rng_)));
		};

		std::function<float(S, A)> U_bound_lambda = [&](S s, A a) -> float {
			return upper_confidence_bound<S, A>(s, a, graph, stats, explore_constant_);
		};

		std::function<Score(S)> initial_value_estimate = [&](S s) -> Score {
			return random_rollout<S, A>(s, graph, rules, random_source);
		};

		mcts_run<S, A>(
			state, stats, graph, rules, sample_actions_uniformly_, U_bound_lambda, random_source, initial_value_estimate);
	}

 private:
	bool sample_actions_uniformly_ = false;
	NonNegative explore_constant_ = NonNegative(1.0F);
	std::mt19937 rng_;
	std::uniform_real_distribution<float> unit_distribution_ = std::uniform_real_distribution<float>(0.0, 1.0);
};

static_assert(std::semiregular<BaseMCTS<int, int>>);

}  // namespace sag::mcts
