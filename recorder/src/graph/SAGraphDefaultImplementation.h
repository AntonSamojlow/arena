#pragma once
#include <memory>
#include <mutex>
#include <random>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "GraphConcepts.h"

namespace graph {

/// <summary>
/// Threadsafe default implementation, relying on a `StateActionRulesEngine`.
/// </summary>
template <typename S, typename A, StateActionRulesEngine<S, A> R>
class SAGraphDefaultImplementation {
 public:
	explicit SAGraphDefaultImplementation(R rules_engine)
			: rules_engine_(rules_engine), roots_(rules_engine.list_roots()) {
		// initialize roots
		std::unique_lock<std::shared_mutex> lock(action_data_mutex_);
		for (S root : roots_) initialize_state_nolock(root);
	}

	auto list_roots() const -> std::vector<S> { return roots_; }
	auto list_actions(S state) const -> std::vector<A> {
		std::shared_lock<std::shared_mutex> lock(action_data_mutex_);
		std::vector<A> result;
		std::ranges::transform(
			action_data_.at(state), std::back_inserter(result), [](const auto& entry) { return entry.first; });
		return result;
	}

	auto list_edges(S state, A action) const -> std::vector<ActionEdge<S>> {
		std::shared_lock<std::shared_mutex> lock(action_data_mutex_);
		return action_data_.at(state).at(action);
	}
	auto score(S state) const -> double { return rules_engine_.score(state); }

	auto is_terminal_at(S state) const -> bool {
		std::shared_lock<std::shared_mutex> lock(action_data_mutex_);
		return action_data_.at(state).empty();
	}

	auto is_expanded_at(S state, A action) const -> bool { return count_edges(state, action) > 0; }
	auto follow(S state, A action, double unit_range_value) const -> S;

	auto count_actions(S state) const -> size_t {
		std::shared_lock<std::shared_mutex> lock(action_data_mutex_);
		return action_data_.at(state).size();
	}

	auto count_edges(S state, A action) const -> size_t {
		std::shared_lock<std::shared_mutex> lock(action_data_mutex_);
		return action_data_.at(state).at(action).size();
	}
	void expand(S state, A action);
	auto get_rules_engine() const -> const R& { return rules_engine_; }

 private:
	using ActionDetails = std::unordered_map<A, std::vector<ActionEdge<S>>>;
	R rules_engine_;
	std::vector<S> roots_;
	std::unordered_map<S, ActionDetails> action_data_;
	mutable std::shared_mutex action_data_mutex_;

	void initialize_state_nolock(S state);
};

template <typename S, typename A, StateActionRulesEngine<S, A> R>
auto SAGraphDefaultImplementation<S, A, R>::follow(S state, A action, double unit_range_value) const -> S {
	if (!is_expanded_at(state, action))
		throw std::logic_error("can not follow an unexpanded action");

#ifdef _DEBUG
	if (unit_range_value < 0.0 || unit_range_value > 1.0)
		throw std::range_error("expected value in [0,1]");
#endif  // _DEBUG

	double sum = 0.0;
	std::vector<ActionEdge<S>> edges = list_edges(state, action);
	auto match = std::ranges::find_if(edges, [&](ActionEdge<S> edge) {
		sum += edge.weight();
		return sum >= unit_range_value;
	});

#ifdef _DEBUG
	if (match == edges.end()) {
		throw std::logic_error("unexpected case during method 'follow': sum (" + std::to_string(sum) +
													 ") does not exceed value (" + std::to_string(unit_range_value) + ")");
	}
#endif  // _DEBUG

	return match->state();
}

template <typename S, typename A, StateActionRulesEngine<S, A> R>
void SAGraphDefaultImplementation<S, A, R>::expand(S state, A action) {
	std::unique_lock<std::shared_mutex> lock(action_data_mutex_);

	// skip if already expanded
	// note: we could promote this to an exception, forcing the calling algorithm to check beforehand (they typically do
	// already...)
	if (action_data_.at(state).at(action).size() > 0)
		return;

	std::vector<ActionEdge<S>> edges = rules_engine_.list_edges(state, action);
	action_data_.at(state).at(action) = edges;
	for (const ActionEdge<S>& edge : edges) initialize_state_nolock(edge.state());
}

// initializes a state by adding all its actions - the caller is responsible of ensuring action_data_ is locked!
template <typename S, typename A, StateActionRulesEngine<S, A> R>
void SAGraphDefaultImplementation<S, A, R>::initialize_state_nolock(S state) {
	if (action_data_.try_emplace(state, ActionDetails()).second) {
		for (A action : rules_engine_.list_actions(state))
			action_data_.at(state).emplace(action, std::vector<ActionEdge<S>>());
	}

	// else do nothing: already initialied (might have been visited via a different parent, repeated init calles, etc.)
}

}  // namespace graph
