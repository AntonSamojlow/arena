#pragma once

#include <tl/expected.hpp>

#include "sag/GraphConcepts.h"
#include "sag/rec/Match.h"
#include "tools/Failure.h"

namespace sag::rec {

// clang-format off
template <typename S, typename State, typename Action>
concept Storage = sag::Vertices<State, Action>
	&& requires (S storage, Match<State, Action> match, std::string_view extra_data){
	{ storage.add(match, extra_data) } -> std::same_as<tl::expected<void, tools::Failure>>;
};

template <class Types>
concept MatchRecorderTypes =
  sag::Graph<typename Types::graph> &&
  Player<typename Types::player, typename Types::graph> &&
	Storage<typename Types::storage, typename Types::graph::state, typename Types::graph::action>;

// clang-format on

}  // namespace sag::rec
