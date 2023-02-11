#pragma once

#include <concepts>
#include <string>
#include <tl/expected.hpp>

#include "Match.h"
#include "sag/Failure.h"
#include "sag/GraphConcepts.h"

namespace sag::match {

// clang-format off

template <typename S, typename State, typename Action>
concept Storage = sag::Vertices<State, Action>
	&& requires (S storage, Match<State, Action> match, std::string json_data){
	{ storage.add(match, json_data) } -> std::same_as<tl::expected<void, Failure>>;
};

// clang-format on

}  // namespace sag::match
