#pragma once
#include <algorithm>
#include <memory>
#include <mutex>
#include <random>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "GraphConcepts.h"

namespace sag {

/*
	{ const_graph.is_terminal_at(state) } -> std::same_as<bool>;
	{ const_graph.is_expanded_at(state, action) } -> std::same_as<bool>;
	{ const_graph.roots() } -> std::same_as<std::vector<S>>;
	{ const_graph.actions_at(state) } -> std::same_as<std::vector<A>>;
	{
		const_graph.edges_at(state, action)
		} -> std::same_as<std::vector<ActionEdge<S>>>;
	{ const_graph.action_count_at(state) } -> std::same_as<size_t>;
	{ const_graph.edge_count_at(state, action) } -> std::same_as<size_t>;
	// non-const operations
	{
		graph.expand_at(state, action, new_edges, next_states)
		} -> std::same_as<bool>;
*/

template <typename S, typename A, RulesEngine<S, A> R>
class DefaultGraphContainer_v1 {
 public:
	explicit DefaultGraphContainer_v1(R&& rules_engine) : rules_engine_(rules_engine){};

	[[nodiscard]] auto is_terminal_at(S state) const -> bool { return data_.empty(); }
	[[nodiscard]] auto is_expanded_at(S state, A action) const -> bool { return edge_count_at(state, action) > 0; }
	[[nodiscard]] auto roots() const -> std::vector<S> { return roots_; }
	[[nodiscard]] auto actions_at(S state) const -> std::vector<A> {
		std::vector<A> result;
		ActionDetails const& action_details = action_data_.at(state);
		result.reserve(action_details.size());
		for (auto const& entry : action_details) {
			result.push_back(entry.first);
		}
		return result;
	}

	[[nodiscard]] auto edges_at(S state, A action) const -> std::vector<ActionEdge<S>> {
		return action_data_.at(state).at(action)
	}

	[[nodiscard]] auto action_count_at(S state) const -> size_t { return action_data_.at(state).size() }

	[[nodiscard]] auto edge_count_at(S state, A action) const -> size_t {
		return action_data_.at(state).at(action).size()
	}

	auto expand_at(
		S state, A action, std::vector<ActionEdge<S>> new_edges, std::unordered_map<S, std::vector<A>> next_states)
		-> bool;

 private:
	R rules_engine_;
	std::vector<S> roots_;
	using ActionDetails = std::unordered_map<A, std::vector<ActionEdge<S>>>;
	std::unordered_map<S, ActionDetails> data_;
};

template <typename S, typename A, RulesEngine<S, A> R>
auto DefaultGraphContainer_v1<S, A, R>::expand_at(
	S state, A action, std::vector<ActionEdge<S>> new_edges, std::unordered_map<S, std::vector<A>> next_states)
	-> bool {

	if (is_expanded_at(state, action))
		return false;

	std::vector<ActionEdge<S>> edges = rules_engine_.list_edges(state, action);
	action_data_.at(state).at(action) = edges;
	for (const ActionEdge<S>& edge : edges) {
		if (action_data_.try_emplace(state).second) {
			for (A action : rules_engine_.list_actions(state))
				action_data_.at(state).emplace(action, std::vector<ActionEdge<S>>());
		}
		// else do nothing: already initialied (possibly visited via a different parent, etc.)
	}
	return true;
}

}  // namespace sag
