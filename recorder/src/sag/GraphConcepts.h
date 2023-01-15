#pragma once

#include <concepts>
#include <string>
#include <utility>
#include <vector>

// disable until clang-format 16 with 'RequiresExpressionIndentation : OuterScope' is available
// clang-format off

namespace sag {

template<typename T>
concept Hashable = requires(T a) {
    { std::hash<T>{}(a) } -> std::convertible_to<std::size_t>;
};

template<typename T>
concept Equatable = requires(T a) {
    { std::equal_to<T>{}(a, a) } -> std::convertible_to<bool>;
};

/// <summary>
/// Types satisfying this concept may be used as keys in std::unordered_map.
/// </summary>
template<typename T>
concept Identifier = std::regular<T> && Hashable<T> && Equatable<T>;

// assert that basic types satisfy Identifier
static_assert(Identifier<char>);
static_assert(Identifier<int>);
static_assert(Identifier<std::string>);
static_assert(false == Identifier<std::vector<int>>);

/// <summary>
/// Wrapper around std::pair, representing an edge weight with the successor state
/// </summary>
template <Identifier State>
struct ActionEdge : public std::pair<double, State> {
	ActionEdge() = default;
	ActionEdge(double&& weight, State&& state) : std::pair<double, State>(weight, state){}

	[[nodiscard]]
	auto weight() const -> double { return this->first; }
	[[nodiscard]]
	auto state() const -> State { return this->second; }

private:
	using std::pair<double, State>::first;
	using std::pair<double, State>::second;
};

static_assert(std::regular<ActionEdge<int>>);
static_assert(std::regular<ActionEdge<std::string>>);

/// <summary>
/// Rules of a state action graph. 
/// CONDITION: the method `list_actions` returns empty when called on a terminal state.
/// </summary>
template <typename R, typename S, typename A> 
concept RulesEngine = requires(R const_rules_engine, S state, A action) {
	{ const_rules_engine.list_roots() } -> std::same_as<std::vector<S>>;
	{ const_rules_engine.list_actions(state) } -> std::same_as<std::vector<A>>;
	{ const_rules_engine.list_edges(state, action) } -> std::same_as<std::vector<ActionEdge<S>>>;
	{ const_rules_engine.score(state) } -> std::same_as<double>;
};

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
template <typename G, typename S, typename A> 
concept Graph =
	RulesEngine<G, S, A> && 
	Identifier<S> && 
	Identifier<A> &&
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
