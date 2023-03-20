#pragma once
#include <spdlog/spdlog.h>

#include <chrono>
#include <random>

#include "Player.h"
#include "sag/GraphConcepts.h"
#include "sag/GraphOperations.h"
#include "tools/Failure.h"

namespace sag::rec {

/// Wrapper around std::pair, representing a state and a correspnding play
template <typename S, typename A>
class Play {
 public:
	Play() = default;
	Play(S const& state, A const& action) : data_{state, action} {}
	Play(S&& state, A&& action) : data_{std::move(state), std::move(action)} {}

	[[nodiscard]] auto state() const -> S { return data_.first; }
	[[nodiscard]] auto action() const -> A { return data_.second; }

#pragma GCC diagnostic push
// reason: https://github.com/llvm/llvm-project/issues/43670
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
	friend auto operator<=>(const Play&, const Play&) = default;
#pragma GCC diagnostic pop

 private:
	std::pair<S, A> data_;
};

static_assert(std::regular<Play<int, int>>);
static_assert(std::regular<Play<int, float>>);

/// Collection of data that represents a match
template <typename S, typename A>
	requires Vertices<S, A>
struct Match {
	std::vector<std::string> player_ids;
	std::chrono::time_point<std::chrono::steady_clock> start = {};
	std::chrono::time_point<std::chrono::steady_clock> end = {};
	std::vector<Play<S, A>> plays;
	S end_state;
	Score end_score;

	friend auto operator<=>(const Match&, const Match&) = default;
};

static_assert(std::regular<Match<int, int>>);
static_assert(std::regular<Match<int, float>>);

template <Graph G>
auto score(Match<typename G::state, typename G::action> const& record, typename G::rules const& rules) -> sag::Score {
	if (record.plays.empty())
		return Score{0};
	return rules.score(record.plays.back());
}

}  // namespace sag::rec
