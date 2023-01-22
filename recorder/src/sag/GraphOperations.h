#pragma once

#include <vector>

#include "BoundedValue.h"
#include "GraphConcepts.h"

namespace sag {

template <typename S>
auto follow(std::vector<ActionEdge<S>> edges, UnitValue random_roll) -> S {
	float threshold = random_roll.value();
	float sum = 0.0F;
	for (auto const& edge : edges) {
		sum += edge.weight().value();
		if (sum >= threshold)
			return edge.state();
	}
	return edges.back().state();
}

template <typename S, typename A, GraphContainer<S, A> G, RulesEngine<S, A> R>
auto expand(G& container, R const& rules, S state, A action) -> bool {
	if (container.is_expanded_at(state, action))
		return false;
	auto new_edges = rules.list_edges(state, action);
	std::vector<std::pair<S, std::vector<A>>> next_states{};
	for (sag::ActionEdge<S> const& edge : new_edges) {
		auto actions = rules.list_actions(edge.state());
		next_states.push_back({edge.state(), actions});
	}
	return container.expand_at(state, action, new_edges, next_states);
}

}  // namespace sag
