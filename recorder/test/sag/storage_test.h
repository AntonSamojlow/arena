#pragma once

#include "sag/match/Match.h"

template <class S, class A>
auto create_plays() -> std::vector<sag::match::Play<S, A>> {
	if constexpr (std::is_arithmetic_v<S>) {
		if constexpr (std::is_arithmetic_v<A>)
			return {{1, 0}, {2, 3}, {3, 1}};
		if constexpr (std::is_same_v<std::string, A>)
			return {{1, "0"}, {2, "3"}, {3, "1"}};
	}

	if constexpr (std::is_same_v<std::string, S>) {
		if constexpr (std::is_arithmetic_v<A>)
			return {{"1", 0}, {"2", 3}, {"3", 1}};
		if constexpr (std::is_same_v<std::string, A>)
			return {{"1", "0"}, {"2", "3"}, {"3", "1"}};
	}
}

template <class S, class A>
auto create_match() -> sag::match::Match<S, A> {
	auto start = std::chrono::steady_clock::now();
	return {
		.player_ids = {"player-1", "player-2", "player-3"},
		.start = start,
		.end = std::chrono::steady_clock::now(),
		.plays = create_plays<S, A>(),
		.end_state = create_plays<S, A>().back().state(),
		.end_score = tools::Score(-1),
	};
}
