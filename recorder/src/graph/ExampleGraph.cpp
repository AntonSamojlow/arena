#include "ExampleGraph.h"

#include <set>

namespace graph::example {
ExampleRulesEngine::ExampleRulesEngine(const GraphStructure& graph_structure) : graph_structure_(graph_structure) {
	// collect all keys
	// if we had ranges, then we could have used `auto keys = graph_structure | std::views::keys;` instead:
	std::vector<State> keys;
	keys.reserve(graph_structure.size());
	for (auto const& entry : graph_structure) keys.push_back(entry.first);

	std::set<State> children;
	// find all children and validate they are defined as key
	for (auto const& entry : graph_structure) {
		for (const ActionEdges& action_edges : entry.second) {
			for (const ActionEdge<State>& edge : action_edges) {
				children.insert(edge.state());
				if (std::find(keys.begin(), keys.end(), edge.state()) == keys.end())
					throw std::invalid_argument("inconsistent initial structure (child node missing in keys)");
			}
		}
	}

	// detemine roots: filter out all children
	for (auto& state : keys) {
		if (!children.contains(state))
			roots_.push_back(state);
	}
}

auto ExampleRulesEngine::list_actions(State state) const -> std::vector<Action> {
	size_t const action_count = graph_structure_.at(state).size();
	std::vector<Action> result;
	result.reserve(action_count);
	for (size_t i = 0; i < action_count; i++) {
		result.push_back(static_cast<Action>(i));
	}
	return result;
}

auto ExampleRulesEngine::list_edges(State state, Action action) const -> ActionEdges {
	return graph_structure_.at(state)[static_cast<size_t>(action)];
}

auto ExampleRulesEngine::score(State state) const -> double {
	return graph_structure_.at(state).empty() ? -1.0 : 0.0;
}
}  // namespace graph::example
