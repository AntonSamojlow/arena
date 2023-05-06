#include "ExampleGraph.h"

#include <fmt/format.h>

#include <set>
#include <utility>

namespace sag::example {

namespace {
void validate_child(Graph::state child, std::vector<Graph::state> const& valid_states) {
	{
		if (std::find(valid_states.begin(), valid_states.end(), child) == valid_states.end())
			throw std::invalid_argument("inconsistent initial structure (child node missing in keys)");
	}
}
}  // namespace

Rules::Rules(const GraphStructure& graph_structure) : graph_structure_(graph_structure) {
	// collect all keys
	// if we had ranges, then we could have used `auto keys = graph_structure | std::views::keys;` instead:
	std::vector<Graph::state> keys;
	keys.reserve(graph_structure.size());
	for (auto const& [state, actions] : graph_structure)
		keys.push_back(state);

	std::set<Graph::state> children;
	// find all children and validate they are defined as key
	for (auto const& [state, actions] : graph_structure) {
		for (const ActionEdges& action_edges : actions) {
			for (const ActionEdge<Graph::state>& edge : action_edges) {
				Graph::state const child = edge.state();
				validate_child(child, keys);
				children.insert(child);
			}
		}
	}

	// detemine roots: filter out all children
	for (auto state : keys) {
		if (!children.contains(state))
			roots_.push_back(state);
	}
}

auto Rules::list_actions(Graph::state state) const -> std::vector<Graph::action> {
	size_t const action_count = graph_structure_.at(state).size();
	std::vector<Graph::action> result;
	result.reserve(action_count);
	for (size_t i = 0; i < action_count; i++) {
		result.push_back(static_cast<Graph::action>(i));
	}
	return result;
}

auto Rules::list_edges(Graph::state state, Graph::action action) const -> ActionEdges {
	return graph_structure_.at(state)[static_cast<size_t>(action)];
}

auto Rules::score(Graph::state state) const -> tools::Score {
	return graph_structure_.at(state).empty() ? tools::Score(-1.0F) : tools::Score(0.0F);
}

auto Container::to_string(Graph::state state) -> std::string {
	return std::to_string(state);
}

auto Container::to_string(Graph::state state, Graph::action action) -> std::string {
	return fmt::format("action-{} at state '{}'", action, state);
}

}  // namespace sag::example
