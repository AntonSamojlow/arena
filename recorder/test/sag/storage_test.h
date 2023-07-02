#pragma once
#include <bitset>

#include "sag/match/Match.h"
#include "sag/santorini/Graph.h"

constexpr sag::santorini::Dimensions SantoriniTestDim;

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

	if constexpr (std::is_same_v<std::bitset<64>, S>) {
		if constexpr (std::is_arithmetic_v<A>)
			return {{std::bitset<64>(1), 0}, {std::bitset<64>(2), 3}, {std::bitset<64>(3), 1}};
	}

	using SanState = sag::santorini::State<SantoriniTestDim>;
	using SanAction = sag::santorini::Action;
	if constexpr (std::is_same_v<SanState, S>) {
		return {{SanState{},
							SanAction{
								.unit_nr = 1,
								.move_location = {.row = 1, .col = 1},
								.build_location = {.row = 2, .col = 2},
							}},
			{SanState{},
				SanAction{
					.unit_nr = 2,
					.move_location = {.row = 1, .col = 2},
					.build_location = {.row = 2, .col = 2},
				}},
			{SanState{},
				SanAction{
					.unit_nr = 1,
					.move_location = {.row = 1, .col = 0},
					.build_location = {.row = 2, .col = 1},
				}}};
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
