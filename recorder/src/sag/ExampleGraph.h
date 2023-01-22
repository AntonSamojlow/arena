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

class ExampleGraph : public sag::DefaultGraphContainer_v1<State, Action> {
 public:
	explicit ExampleGraph(ExampleRulesEngine&& rules) : sag::DefaultGraphContainer_v1<State, Action>(std::move(rules)) {}
	explicit ExampleGraph(ExampleRulesEngine const& rules) : sag::DefaultGraphContainer_v1<State, Action>(rules) {}
	ExampleGraph() : ExampleGraph(ExampleRulesEngine({})) {}

	static auto stringify(State state) -> std::string { return std::to_string(state); }
	static auto stringify(State state, Action action) -> std::string {
		return fmt::format("action-{} at state '{}'", action, state);
	}
};

static_assert(GraphContainer<ExampleGraph, State, Action>);
static_assert(VertexStringifier<ExampleGraph, State, Action>);

}  // namespace sag::example
