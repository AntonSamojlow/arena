#pragma once

#include <fmt/format.h>

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

}  // namespace sag::match
