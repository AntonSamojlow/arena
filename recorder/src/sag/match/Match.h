#pragma once
#include <spdlog/spdlog.h>

#include <chrono>
#include <random>
#include <vector>

#include "Player.h"
#include "sag/GraphConcepts.h"
#include "sag/GraphOperations.h"

namespace sag::match {

/// Wrapper around std::pair, representing a state and a correspnding play
template <Graph G>
class Play {
 public:
	Play() = default;
	Play(typename G::state const& state, typename G::action const& action) : data_{state, action} {}
	Play(typename G::state&& state, typename G::action&& action) : data_{std::move(state), std::move(action)} {}

	[[nodiscard]] auto state() const -> typename G::state { return data_.first; }
	[[nodiscard]] auto action() const -> typename G::action { return data_.second; }

#pragma GCC diagnostic push
// reason: https://github.com/llvm/llvm-project/issues/43670
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
	friend auto operator<=>(const Play&, const Play&) = default;
#pragma GCC diagnostic pop

 private:
	std::pair<typename G::state, typename G::action> data_;
};

// static_assert(std::regular<Play<sag::tic_tac_toe::Graph>>);

/// Collection of data that represents a match
template <Graph G>
struct Match {
	std::chrono::time_point<std::chrono::steady_clock> start = {};
	std::chrono::time_point<std::chrono::steady_clock> end = {};
	std::vector<Play<G>> plays;
	typename G::state end_state;

	friend auto operator<=>(const Match&, const Match&) = default;
};

// static_assert(std::regular<Match<sag::tic_tac_toe::Graph>>);

class MatchRecorder {
 public:
	MatchRecorder() {
		std::random_device rd;
		rng_ = std::mt19937(rd());
	}

	template <Graph G>
	auto record_duel(typename G::state root,
		typename G::container& graph,
		typename G::rules const& rules,
		Player<G> auto& first_player,
		Player<G> auto& second_player) -> Match<G> {
		// validate the root
		auto roots = graph.roots();
		if (std::ranges::find(roots, root) == roots.end())
			return {};

		logger_->debug("match starts");
		Match<G> match{.start = std::chrono::steady_clock::now(), .end = {}, .plays = {}, .end_state = {}};
		typename G::state state = root;

		for (size_t turn = 0; !graph.is_terminal_at(state); ++turn) {
			logger_->debug("turn {}", turn);
			typename G::action action =
				turn % 2 == 0 ? first_player.play(state, graph, rules) : second_player.play(state, graph, rules);
			match.plays.emplace_back(state, action);
			if (!graph.is_expanded_at(state, action))
				sag::expand(graph, rules, state, action);

			state = sag::follow(graph.edges_at(state, action), UnitValue{unit_distribution_(rng_)});
		}
		match.end = std::chrono::steady_clock::now();
		match.end_state = state;
		logger_->debug("match ends");
		return match;
	}

 private:
	std::shared_ptr<spdlog::logger> logger_ = spdlog::default_logger();
	std::mt19937 rng_;
	std::uniform_real_distribution<float> unit_distribution_ = std::uniform_real_distribution<float>(0.0, 1.0);
};

static_assert(std::semiregular<MatchRecorder>);

template <Graph G>
auto score(Match<G> const& record, typename G::rules const& rules) -> sag::Score {
	if (record.plays.empty())
		return Score{0};
	return rules.score(record.plays.back());
}

}  // namespace sag::match
