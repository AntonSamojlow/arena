#pragma once
#include <map>
#include <string>
#include <utility>

#include "GraphConcepts.h"
#include "SAGraphDefaultImplementation.h"

namespace graph::example {

using State = int;
using Action = int;

using ActionEdges = std::vector<ActionEdge<State>>;

using GraphStructure = std::map<State, std::vector<ActionEdges>>;

class ExampleRulesEngine {
 public:
	explicit ExampleRulesEngine(GraphStructure graph_structure);

	[[nodiscard]] auto list_roots() const -> std::vector<State> { return roots_; }
	[[nodiscard]] auto list_actions(State state) const -> std::vector<Action>;
	[[nodiscard]] auto list_edges(State state, Action action) const -> std::vector<ActionEdge<State>>;
	[[nodiscard]] auto score(State state) const -> double;

 private:
	std::vector<State> roots_;
	GraphStructure graph_structure_;
};

class ExampleGraph : public graph::SAGraphDefaultImplementation<State, Action, ExampleRulesEngine> {
 public:
	explicit ExampleGraph(ExampleRulesEngine rules_engine)
			: graph::SAGraphDefaultImplementation<State, Action, ExampleRulesEngine>(std::move(rules_engine)) {}

	static auto stringify(State state) -> std::string { return std::to_string(state); }
	static auto stringify(State state, Action action) -> std::string {
		return "action-" + std::to_string(action) + "at state '" + std::to_string(state) + "'";
	}
};

}  // namespace graph::example
