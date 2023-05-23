#pragma once

#include <random>
#include <string_view>

#include "sag/GraphConcepts.h"
#include "tools/BoundedValue.h"

namespace sag::match {

/// ABC for players
template <Graph G>
class Player {
 public:
	virtual ~Player() = default;
	Player() = default;

	Player(const Player&) = default;
	Player(Player&&) noexcept = default;
	auto operator=(const Player&) -> Player& = default;
	auto operator=(Player&&) noexcept -> Player& = default;

	Player(std::string_view id, std::string_view name) : id_(id), name_(name) {}  // NOLINT(*swappable-parameters)

	[[nodiscard]] auto display_name() const -> std::string_view { return id_; }
	[[nodiscard]] auto id() const -> std::string_view { return name_; }

	[[nodiscard]] virtual auto choose_play(
		typename G::state state, typename G::container& graph, typename G::rules const& rules) -> typename G::action = 0;

	auto operator==(const Player& other) const -> bool = default;

 private:
	std::string id_;
	std::string name_;
};

/// Helper class to implement a player with access to RNG
template <Graph G>
class ProbabilisticPlayer : public Player<G> {
 public:
	ProbabilisticPlayer(std::string_view id, std::string_view name)
			: Player<G>(id, name) {}  // NOLINT(*swappable-parameters)

	auto operator==(const ProbabilisticPlayer& other) const -> bool = default;

 protected:
	[[nodiscard]] auto roll() -> tools::UnitValue { return tools::UnitValue{unit_distribution_(rng_)}; }

 private:
	std::mt19937 rng_ = std::mt19937(std::random_device()());
	std::uniform_real_distribution<float> unit_distribution_ = std::uniform_real_distribution<float>(0.0, 1.0);
};

}  // namespace sag::match
