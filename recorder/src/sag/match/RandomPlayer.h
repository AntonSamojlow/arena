#pragma once

#include <limits>
#include <random>

#include "Player.h"
#include "sag/ExampleGraph.h"

namespace sag::match {

template <Graph G>
class RandomPlayer {
 public:
	RandomPlayer() = default;
	RandomPlayer(std::string_view name, std::string_view id)  // NOLINT(*swappable-parameters)
			: rng_(std::mt19937(std::random_device()())), id_(id), name_(name) {}

	[[nodiscard]] auto id() const -> std::string { return id_; }
	[[nodiscard]] auto display_name() const -> std::string { return name_; }

	auto operator==(const RandomPlayer& other) const -> bool = default;

	[[nodiscard]] auto choose_play(
		typename G::state state, typename G::container& graph, typename G::rules const& /*unused*/) -> typename G::action {
		std::vector<typename G::action> const actions = graph.actions_at(state);
		auto random_index = static_cast<size_t>(std::floor(unit_distribution_(rng_) * static_cast<float>(actions.size())));
		return actions[random_index];
	}

 private:
	std::mt19937 rng_;
	std::uniform_real_distribution<float> unit_distribution_ = std::uniform_real_distribution<float>(0.0, 1.0);
	std::string id_ = "default(random)";
	std::string name_ = "random-player";
};

static_assert(Player<RandomPlayer<sag::example::Graph>, sag::example::Graph>);

}  // namespace sag::match
