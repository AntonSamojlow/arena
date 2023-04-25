#pragma once

#include <fmt/format.h>

#include <random>

#include "sag/ExampleGraph.h"
#include "sag/GraphConcepts.h"

namespace sag::match {

// clang-format off
template <typename P, typename G>
concept Player = std::regular<P> && Graph<G> &&
requires(P player,
	P const c_player,
	typename G::state state,
	typename G::container& ref_container,
	typename G::rules const& cref_rules) {
	{ c_player.display_name() } -> std::same_as<std::string>;
	{ c_player.id() } -> std::same_as<std::string>;
	{ player.choose_play(state, ref_container, cref_rules) } -> std::same_as<typename G::action>;
};

// clang-format on

template <sag::Graph G>
auto player_id(Player<G> auto const& player) -> std::string {
	return fmt::format("{}:{}", player.name, player.version);
}

template <Graph G>
class RandomPlayer {
 public:
	RandomPlayer() {
		std::random_device rand;
		rng_ = std::mt19937(rand());
	}

	[[nodiscard]] auto id() const -> std::string { return name_; }
	[[nodiscard]] auto display_name() const -> std::string { return name_; }

	auto operator==(const RandomPlayer& other) const -> bool { return name_ == other.name_; }

	[[nodiscard]] auto choose_play(
		typename G::state state, typename G::container& graph, typename G::rules const& /*unused*/) -> typename G::action {
		std::vector<typename G::action> const actions = graph.actions_at(state);
		auto random_index = static_cast<size_t>(std::floor(unit_distribution_(rng_) * static_cast<float>(actions.size())));
		return actions[random_index];
	}

 private:
	std::string name_ = "random player";
	std::mt19937 rng_;
	std::uniform_real_distribution<float> unit_distribution_ = std::uniform_real_distribution<float>(0.0, 1.0);
};

static_assert(Player<RandomPlayer<sag::example::Graph>, sag::example::Graph>);

}  // namespace sag::match
