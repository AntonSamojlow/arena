#pragma once

#include <fmt/format.h>

#include <random>

#include "Player.h"

namespace sag::match {

template <Graph G>
class RandomPlayer {
 public:
	RandomPlayer() : rng_(std::mt19937(std::random_device()())) {}
	explicit RandomPlayer(std::string_view name) : rng_(std::mt19937(std::random_device()())), name_(name) {}

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
	std::mt19937 rng_;
	std::uniform_real_distribution<float> unit_distribution_ = std::uniform_real_distribution<float>(0.0, 1.0);
	std::string name_ = "random-player";
};

static_assert(Player<RandomPlayer<sag::example::Graph>, sag::example::Graph>);

}  // namespace sag::match
