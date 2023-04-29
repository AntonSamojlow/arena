#pragma once
#include <tl/expected.hpp>

#include "Player.h"
#include "sag/GraphOperations.h"
#include "tools/Failure.h"

namespace sag::match {

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
	tools::Score end_score;

	friend auto operator<=>(const Match&, const Match&) = default;
};

static_assert(std::regular<Match<int, int>>);
static_assert(std::regular<Match<int, float>>);

template <Graph G>
auto score(Match<typename G::state, typename G::action> const& record, typename G::rules const& rules) -> tools::Score {
	if (record.plays.empty())
		return tools::Score{0};
	return rules.score(record.plays.back());
}

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

}  // namespace sag::match
