#pragma once

#include <concepts>
#include <string>
#include <utility>
#include <vector>

// disable until clang-format 16 with 'RequiresExpressionIndentation : OuterScope' is available
// clang-format off

namespace graph {

/// <summary>
/// Wrapper around std::pair, representing an edge weight with the successor state
/// </summary>
template <typename S>
struct ActionEdge : private std::pair<double, S> {
	ActionEdge(double&& weight, S&& state) : std::pair<double, S>(weight, state){}

	[[nodiscard]]
	auto weight() const -> double { return this->first; }
	[[nodiscard]]
	auto state() const -> S { return this->second; }

private:
	using std::pair<double, S>::first;
	using std::pair<double, S>::second;
};

/// <summary>
/// Rules of a state action graph. 
/// CONDITION: the method `list_actions` returns empty when called on a terminal state.
/// </summary>
template <typename R, typename S, typename A> concept StateActionRulesEngine = 
requires(R const_rules_engine, S state, A action) {
	{ const_rules_engine.list_roots() } -> std::same_as<std::vector<S>>;
	{ const_rules_engine.list_actions(state) } -> std::same_as<std::vector<A>>;
	{ const_rules_engine.list_edges(state, action) } -> std::same_as<std::vector<ActionEdge<S>>>;
	{ const_rules_engine.score(state) } -> std::same_as<double>;
};

template<typename T>
concept Hashable = requires(T a) {
    { std::hash<T>{}(a) } -> std::convertible_to<std::size_t>;
};

template<typename T>
concept Equatable = requires(T a) {
    { std::equal_to<T>{}(a, a) } -> std::convertible_to<bool>;
};

template<typename T>
concept UnorderedMappable = Equatable<T> && Hashable<T>;

/// <summary>
/// A rooted directed graph with two types of vertices, 'states' and 'actions'.
/// CONDITIONS:
/// Both state and actions must allow to be used as key in std::unordered_map (see concept UnorderedMappable).
/// Leaving edges of states end at actions. Leaving edges of actions are weighted, sum to 1 and end at states.
/// The following **guarantees are expected** from each implementation:
/// 1. Each state has 0+ leaving edges and this count may not change (for known / initialized states).
///	2. Each states leaving edges are ordered and indexed, starting from 0.
/// 3. States are terminal iff they have 0 leaving edges.
/// 4. The count of leaving edges of an action may be 1+ or unknown (not 0).
/// 5. An action is *expanded* iff the number of leaving edges is known. This count never changes, once known.
/// 6. Expanding an action determines all leaving (weighted) edges with their destination states (including the leaving
/// edge count of each destination state).
/// </summary>
template <typename G, typename S, typename A> concept StateActionGraph =
	StateActionRulesEngine<G, S, A> && 
	UnorderedMappable<S> && 
	UnorderedMappable<A> &&
	requires(G graph, G const const_graph, S state, A action, double unit_range_value) {
		{ const_graph.is_terminal_at(state) } -> std::same_as<bool>;
		{ const_graph.is_expanded_at(state, action) } -> std::same_as<bool>;
		{ const_graph.follow(state, action, unit_range_value) } -> std::same_as<S>;

		{ const_graph.count_actions(state) } -> std::same_as<size_t>;
		{ const_graph.count_edges(state, action) } -> std::same_as<size_t>;

		{ graph.expand(state, action) } -> std::same_as<void>;
	};

template <typename N, typename S, typename A>
concept NodeSerializer = requires(N const const_serializer, S state, A action) {
	{ const_serializer.stringify(state) } -> std::same_as<std::string>;
	{ const_serializer.stringify(state, action) } -> std::same_as<std::string>;
};

}  // namespace graph
