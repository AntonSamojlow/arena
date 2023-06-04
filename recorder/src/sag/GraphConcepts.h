#pragma once

#include <string>
#include <utility>
#include <vector>

#include "tools/BoundedValue.h"

// disable until clang-format 16 with 'RequiresExpressionIndentation : OuterScope' is available
// clang-format off

/// sag: A state-action graph is a rooted directed graph with two types of vertices, 'states' and 'actions'.
/// It satisfies:
/// 1. Each state has 0+ leaving edges which end at actions.
/// 2. A state is called 'terminal' iff it has 0 leaving edges.
/// 3. Each edge has 1+ leaving weighted edges that sum to 1 and end at states.
namespace sag {

// --------------------------------------------------------------------------------------------------------------------
// Helper concepts and types
// --------------------------------------------------------------------------------------------------------------------

template <typename T>
concept Hashable = requires(T type) {
	{ std::hash<T>{}(type) } -> std::convertible_to<std::size_t>;
};

template <typename T>
concept Equatable = requires(T type) {
	{ std::equal_to<T>{}(type, type) } -> std::convertible_to<bool>;
};

/// Types satisfying this concept may be used as keys in std::unordered_map.
template <typename T> concept Identifier = std::regular<T> && Hashable<T> && Equatable<T>;

// assert that basic types satisfy Identifier
static_assert(Identifier<char>);
static_assert(Identifier<int>);
static_assert(Identifier<std::string>);
// sanity check that STL containers do not satisfy Identifier
static_assert(!Identifier<std::vector<int>>);
static_assert(!Identifier<std::pair<int, int>>);

template <typename StateId, typename ActionId>
/// The types used to identify states and actions of a state-action graph.
/// REQUIREMENT:
/// - a state id is globally unique for each state
/// - an action id may not be unique for each action,
/// it is only required to be unique among the actions of each individual state
concept Vertices = Identifier<StateId> && Identifier<ActionId>;

/// Wrapper around std::pair, representing an edge weight with the successor state
template <Identifier State>
class ActionEdge{
public:
	ActionEdge() = default;
	ActionEdge(float weight, State const& state) : data_{ weight, state} {}
	ActionEdge(float weight, State&& state) : data_{ weight, std::move(state) } {}

	[[nodiscard]] auto weight() const -> tools::UnitValue { return data_.first; }
	[[nodiscard]] auto state() const -> State { return data_.second; }

	friend auto operator<=>(const ActionEdge<State>&, const ActionEdge<State>&) = default;

 private:
	std::pair<tools::UnitValue, State> data_;
};

static_assert(std::regular<ActionEdge<int>>);
static_assert(std::regular<ActionEdge<double>>);
static_assert(std::regular<ActionEdge<std::string>>);

// --------------------------------------------------------------------------------------------------------------------
// Main graph concepts
// --------------------------------------------------------------------------------------------------------------------

template <typename G, typename S, typename A>
/// Container for a state-action graph.
/// The base modifiying operation is an *expansion* of a state-action which adds
/// all leaving (weighted) edges with their destination states, including their non-expanded actions.
/// REQUIREMENTS:
/// - 'is_expanded_at' is true iff the container knows the leaving edges of the state-action (this pair 'is expanded').
/// - 'edges_at' for a non-expanded state-action returns emtpy.
/// - 'expand_at' for an expanded state-action a no-op which returns false.
/// - 'expand_at' returns false iff the graph failed to add the new edges and next states to itself.
concept GraphContainer = Vertices<S, A> && std::regular<G> && requires(G graph,
	G const const_graph,
	S state,
	A action,
	std::vector<A> actions,
	std::vector<ActionEdge<S>> new_edges,
	std::vector<std::pair<S, std::vector<A>>> next_states) {

	// const operations
	{ const_graph.is_terminal_at(state) } -> std::same_as<bool>;
	{ const_graph.is_expanded_at(state, action) } -> std::same_as<bool>;
	{ const_graph.roots() } -> std::same_as<std::vector<S>>;
	{ const_graph.actions_at(state) } -> std::same_as<std::vector<A>>;
	{ const_graph.edges_at(state, action) } -> std::same_as<std::vector<ActionEdge<S>>>;
	{ const_graph.action_count_at(state) } -> std::same_as<size_t>;
	{ const_graph.edge_count_at(state, action) } -> std::same_as<size_t>;

	// non-const operations
	{ graph.expand_at(state, action, new_edges, next_states) } -> std::same_as<bool>;
	{ graph.add(state, actions) } -> std::same_as<bool>;
};

template <typename R, typename S, typename A>
/// Generates roots, actions and edges for a state-action graph.
/// REQUIREMENTS:
/// - 'list_roots' returns always the same 1+ states in the same order.
/// - For each state, 'list_actions' returns always the same 0+ actions in the same order.
/// - 'list_actions' may return empty for a state, but then 'score' must return either +1 or -1 for this state (it is
/// terminal).
/// - For each state-action pair, 'list_edges' returns always the same 1+ action edges in the same order.
concept RulesEngine = Vertices<S, A> && requires(R const const_rules_engine, S state, A action) {
	{ const_rules_engine.list_roots() } -> std::same_as<std::vector<S>>;
	{ const_rules_engine.list_actions(state) } -> std::same_as<std::vector<A>>;
	{ const_rules_engine.list_edges(state, action) } -> std::same_as<std::vector<ActionEdge<S>>>;
	{ const_rules_engine.score(state) } -> std::same_as<tools::Score>;
};

template <typename G, typename S, typename A>
/// A graph container that has a count of the states, actions and (action) edges
/// it *currently* holds (not the total count of all possible entities).
concept CountingGraphContainer = GraphContainer<G, S, A> && requires(G const const_graph, S state, A action) {
	{ const_graph.action_count() } -> std::same_as<size_t>;
	{ const_graph.state_count() } -> std::same_as<size_t>;
	{ const_graph.edge_count() } -> std::same_as<size_t>;
};

template <typename N, typename S, typename A>
concept VertexPrinter = Vertices<S, A> && requires(N const const_stringifier, S state, A action) {
	{ const_stringifier.to_string(state) } -> std::same_as<std::string>;
	{ const_stringifier.to_string(state, action) } -> std::same_as<std::string>;
};

/// Collection of types for a specific state action graph
template <typename G>
concept Graph =
	sag::GraphContainer<typename G::container, typename G::state, typename G::action> &&
	sag::RulesEngine<typename G::rules, typename G::state, typename G::action> &&
	sag::VertexPrinter<typename G::printer, typename G::state, typename G::action>;

}  // namespace sag
