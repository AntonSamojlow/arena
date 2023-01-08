#pragma once

#include <concepts>
#include <string>
#include <utility>
#include <vector>

// disable until clang-format 16 with 'RequiresExpressionIndentation : OuterScope' is available
// clang-format off

namespace graph {

/// <summary>
/// Wrapper around std::pair<double, S>, representing an edge weight with the successor state
/// </summary>
template <typename S>
struct ActionEdge : private std::pair<double, S> {
	ActionEdge(double weight, S state) : std::pair<double, S>(weight, state){}

	[[nodiscard]]
	auto weight() const -> double { return this->first; }
	[[nodiscard]]
	auto state() const -> S { return this->second; }
};

/// <summary>
/// Rules of a state action graph. Note that `list_actions` for a terminal state should return empty.
/// </summary>
template <typename R, typename S, typename A>
concept StateActionRulesEngine = requires(R const_rules_engine, S state, A action) {
	{ const_rules_engine.list_roots() } -> std::convertible_to<std::vector<S>>;
	{ const_rules_engine.list_actions(state) } -> std::convertible_to<std::vector<A>>;
	{
		const_rules_engine.list_edges(state, action)
		} -> std::convertible_to<std::vector<ActionEdge<S>>>;
	{ const_rules_engine.score(state) } -> std::convertible_to<double>;
};

/// <summary>
/// A rooted directed graph with two types of vertices, 'states' and 'actions'.
///
/// The state must allow storage in an UnorderedAssociativeContainer (STL).
/// Leaving edges of states end at actions. Leaving edges of actions are weighted, sum to 1 and end at states.
///
/// The following **guarantees are expected** from each implementation :
/// 1. each state has 0+ leaving edges and this count may not change (for known / initialized states)
///	2. each states leaving edges are ordered and indexed, starting from 0
/// 3. states are terminal iff they have 0 leaving edges
/// 4. the count of leaving edges of an action may be 1+ or unknown
/// 5. an action is *expanded* iff the number of leaving edges is known and this count does never change, once known
/// 6. expanding an action determines all leaving weighted edges with their destination states (including the leaving
/// edge count of each destination state)
/// </summary>
template <typename G, typename S, typename A>
concept StateActionGraph =
	StateActionRulesEngine<G, S, A> &&
	requires(G graph, G const const_graph, S state, A action, double unit_range_value) {
		{ const_graph.is_terminal_at(state) } -> std::convertible_to<bool>;
		{ const_graph.is_expanded_at(state, action) } -> std::convertible_to<bool>;
		{ const_graph.follow(state, action, unit_range_value) } -> std::convertible_to<S>;

		{ const_graph.count_actions(state) } -> std::convertible_to<size_t>;
		{ const_graph.count_edges(state, action) } -> std::convertible_to<size_t>;

		{ graph.expand(state, action) } -> std::same_as<void>;
	};

template <typename N, typename S, typename A>
concept NodeSerializer = requires(N const const_serializer, S state, A action) {
	{ const_serializer.stringify(state) } -> std::convertible_to<std::string>;
	{ const_serializer.stringify(state, action) } -> std::convertible_to<std::string>;
};

}  // namespace graph
