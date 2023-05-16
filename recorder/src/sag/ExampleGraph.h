#pragma once

#include <map>
#include <string>

#include "DefaultGraphContainer_v1.h"

namespace sag::example {

class Container;
class Rules;

struct Graph {
	using state = int;
	using action = int;
	using container = Container;
	using rules = Rules;
	using printer = Container;
};

using ActionEdges = std::vector<ActionEdge<Graph::state>>;
using GraphStructure = std::map<Graph::state, std::vector<ActionEdges>>;

/// A generic example of a rules engine, based on a provided *fixed* graph structure.
class Rules {
 public:
	explicit Rules(const GraphStructure& graph_structure);

	[[nodiscard]] auto list_roots() const -> std::vector<Graph::state> { return roots_; }
	[[nodiscard]] auto list_actions(Graph::state state) const -> std::vector<Graph::action>;
	[[nodiscard]] auto list_edges(Graph::state state, Graph::action action) const
		-> std::vector<ActionEdge<Graph::state>>;
	[[nodiscard]] auto score(Graph::state state) const -> tools::Score;

 private:
	std::vector<Graph::state> roots_;
	GraphStructure graph_structure_;
};

static_assert(RulesEngine<Rules, Graph::state, Graph::action>);

/// An example graph container (default graph container, backed by an example rules engine)
class Container : public sag::DefaultGraphContainer_v1<Graph::state, Graph::action> {
	using sag::DefaultGraphContainer_v1<Graph::state, Graph::action>::DefaultGraphContainer_v1;

 public:
	Container() : Container(Rules({})) {}

	static auto to_string(Graph::state state) -> std::string;
	static auto to_string(Graph::state state, Graph::action action) -> std::string;
};

static_assert(sag::Graph<Graph>);

}  // namespace sag::example
