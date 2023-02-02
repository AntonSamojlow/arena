#pragma once
// #include <random>
#include <functional>
#include <iterator>
#include <random>

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
class MCTS {
 public:
	MCTS() {
		std::random_device rd;
		rng_ = std::mt19937(rd());
	}

	MCTS(bool sample_actions_uniformly, unsigned int explore_constant) : MCTS() {
		sample_actions_uniformly_ = sample_actions_uniformly;
		explore_constant_ = explore_constant;
	}

	auto descend(S state,
		StatsContainer<S> auto& stats,
		sag::GraphContainer<S, A> auto& graph,
		sag::RulesEngine<S, A> auto const& rules) -> void {
		// ensure start state is listed
		if (!stats.has(state))
			stats.initialize(state, rules.score(state));

		std::function<float(S, A)> U_bound_lambda = [&](S state, A action) -> float {
			return U_bound(state, action, graph, stats);
		};

		std::function<float(S)> initial_value_lambda = [&](S state) -> float {
			return initial_value_estimate(state, graph, rules, random_source);
		};

		Path<S> const path = select(state, graph, sample_actions_uniformly_, U_bound_lambda, random_source);
		expand(path, stats, graph, rules, initial_value_lambda);
		update(path, stats);
	}

 private:
	bool sample_actions_uniformly_ = true;
	unsigned int explore_constant_ = 2;
	std::mt19937 rng_;
	std::uniform_real_distribution<float> unit_distribution_ = std::uniform_real_distribution<float>(0.0, 1.0);

	auto random_source() -> float { return unit_distribution_(rng_); }

	auto initial_value_estimate(S state,
		sag::GraphContainer<S, A> auto& graph,
		sag::RulesEngine<S, A> auto const& rules,
		std::function<UnitValue(void)> random_source) const -> float {
		int rollout_length = 0;
		while (!graph.is_terminal_at(state)) {
			// pick a random action, expand and follow
			std::vector<A> actions = graph.actions_at(state);
			size_t index = static_cast<size_t>(std::floor(actions.size() * random_source()));
			A action = actions[index];
			if (!graph.is_expanded_at(state, action)) {
				sag::expand(graph, rules, state, action);
			}
			state = sag::follow(graph.edges_at(state, action), random_source());
			rollout_length++;
		}
		float value = (1 - 2 * (rollout_length % 2)) * rules.score(state);
		return value;
	}

	auto U_bound(S state, A action, sag::GraphContainer<S, A> auto& graph, StatsContainer<S> auto& stats) const -> float {
		float value_estimate = 0.0;
		int action_visits = 0;
		for (ActionEdge egde : graph.edges_at(state, action)) {
			value_estimate += (egde.weight() * stats.at(egde.state()).Q);
			action_visits += stats.at(egde.state()).N;
		}

		return value_estimate - explore_constant_ * std::sqrt(stats.at(state).N / (1 + action_visits));
	}
};

static_assert(std::semiregular<MCTS<int, int>>);

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
