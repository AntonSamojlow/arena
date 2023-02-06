#pragma once
#include <fmt/format.h>

#include <map>
#include <string>
#include <utility>

#include "DefaultGraphContainer_v1.h"
#include "GraphConcepts.h"

namespace sag::example {

using State = int;
using Action = int;
using ActionEdges = std::vector<ActionEdge<State>>;
using GraphStructure = std::map<State, std::vector<ActionEdges>>;

/// A generic example of a rules engine, based on a provided *fixed* graph structure.
class ExampleRulesEngine {
 public:
	explicit ExampleRulesEngine(const GraphStructure& graph_structure);

	[[nodiscard]] auto list_roots() const -> std::vector<State> { return roots_; }
	[[nodiscard]] auto list_actions(State state) const -> std::vector<Action>;
	[[nodiscard]] auto list_edges(State state, Action action) const -> std::vector<ActionEdge<State>>;
	[[nodiscard]] auto score(State state) const -> Score;

 private:
	std::vector<State> roots_;
	GraphStructure graph_structure_;
};

static_assert(RulesEngine<ExampleRulesEngine, State, Action>);

/// An example graph container (default graph container backed by an example ruels engine)
class ExampleGraph : public sag::DefaultGraphContainer_v1<State, Action> {
	using sag::DefaultGraphContainer_v1<State, Action>::DefaultGraphContainer_v1;

 public:
	ExampleGraph() : ExampleGraph(ExampleRulesEngine({})) {}

	static auto to_string(State state) -> std::string;
	static auto to_string(State state, Action action) -> std::string;
};

static_assert(GraphContainer<ExampleGraph, State, Action>);
static_assert(VertexPrinter<ExampleGraph, State, Action>);

}  // namespace sag::example
