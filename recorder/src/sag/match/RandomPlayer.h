#pragma once

#include <concepts>
#include <limits>

#include "Player.h"
#include "sag/ExampleGraph.h"
#include "sag/match/Player.h"

namespace sag::match {

template <Graph G>
class RandomPlayer : public ProbabilisticPlayer<G> {
 public:
	RandomPlayer() : ProbabilisticPlayer<G>("default(random)", "random-player") {}
	RandomPlayer(std::string_view name, std::string_view id)  // NOLINT(*swappable-parameters)
			: ProbabilisticPlayer<G>(id, name) {}

	auto operator==(const RandomPlayer& other) const -> bool = default;

	[[nodiscard]] auto choose_play(
		typename G::state state, typename G::container& graph, typename G::rules const& /*unused*/) ->
		typename G::action override {
		std::vector<typename G::action> const actions = graph.actions_at(state);
		size_t random_index = std::min(actions.size() - 1,
			static_cast<size_t>(std::floor(ProbabilisticPlayer<G>::roll().value() * static_cast<float>(actions.size()))));
		return actions[random_index];
	}
};

static_assert(std::regular<RandomPlayer<sag::example::Graph>>);

}  // namespace sag::match
