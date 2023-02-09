#pragma once

#include <random>

#include "sag/GraphConcepts.h"
#include "sag/TicTacToe.h"

namespace sag::match {

template <typename P, typename G>
concept Player =
	Graph<G> &&
	requires(
		P player, typename G::state state, typename G::container& ref_container, typename G::rules const& cref_rules) {
		{ player.id } -> std::convertible_to<std::string>;
		{ player.name } -> std::convertible_to<std::string>;
		{ player.play(state, ref_container, cref_rules) } -> std::same_as<typename G::action>;
	};

template <Graph G>
class RandomPlayer {
 public:
	RandomPlayer() {
		std::random_device rand;
		rng_ = std::mt19937(rand());
	}

	std::string id;
	std::string name;

	auto play(typename G::state state, typename G::container& graph, typename G::rules const& /*unused*/) ->
		typename G::action {
		std::vector<typename G::action> const actions = graph.actions_at(state);
		auto random_index = static_cast<size_t>(std::floor(unit_distribution_(rng_) * static_cast<float>(actions.size())));
		return actions[random_index];
	}

 private:
	std::mt19937 rng_;
	std::uniform_real_distribution<float> unit_distribution_ = std::uniform_real_distribution<float>(0.0, 1.0);
};

static_assert(Player<RandomPlayer<sag::tic_tac_toe::Graph>, sag::tic_tac_toe::Graph>);

}  // namespace sag::match
