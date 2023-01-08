#include "ExampleGraph.h"

#include <vcruntime.h>

#include <ranges>
#include <set>

namespace graph::example {
ExampleRulesEngine::ExampleRulesEngine(GraphStructure graph_structure) : graph_structure_(graph_structure) {
	// find all children and validate the nare defined as key
	auto keys = graph_structure | std::views::keys;
	std::set<State> children;
	for (const std::vector<ActionEdges>& actions : graph_structure | std::views::values) {
		for (const ActionEdges& action_edges : actions) {
			for (const ActionEdge<State>& edge : action_edges) {
				children.insert(edge.state());
				if (std::ranges::find(keys, edge.state()) == keys.end())
					throw std::invalid_argument("inconsistent initial structure (child node missing in keys)");
			}
		}
	}

	// detemine roots: filter out all children
	auto root_view = graph_structure | std::views::keys | std::views::filter([&](const State state) {
		return std::ranges::find(children, state) == children.end();
	});

	// populate roots container
	std::ranges::copy(root_view, std::back_inserter(roots_));
}

auto ExampleRulesEngine::list_actions(State state) const -> std::vector<Action> {
	auto action_view = std::views::iota(0, static_cast<int>(graph_structure_.at(state).size()));
	return {action_view.begin(), action_view.end()};
}

auto ExampleRulesEngine::list_edges(State state, Action action) const -> ActionEdges {
	return graph_structure_.at(state)[static_cast<size_t>(action)];
}

auto ExampleRulesEngine::score(State state) const -> double {
	return graph_structure_.at(state).empty() ? -1.0 : 0.0;
}
}  // namespace graph::example
