#pragma once
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <chrono>
#include <concepts>
#include <iterator>
#include <memory>
#include <random>
#include <stop_token>
#include <tl/expected.hpp>
#include <type_traits>

#include "Match.h"
#include "MatchConcepts.h"
#include "sag/GraphConcepts.h"
#include "sag/GraphOperations.h"
#include "sag/TicTacToe.h"
#include "sag/rec/MatchConcepts.h"
#include "sag/rec/Player.h"
#include "sag/rec/Recorder.h"
#include "sag/storage/MemoryMatchStorage.h"
#include "tools/MutexQueue.h"

namespace sag::rec {

struct TicTacToeRecorder {
	using graph = tic_tac_toe::Graph;
	using player = RandomPlayer<tic_tac_toe::Graph>;
	using storage = storage::MemoryMatchStorage<typename graph::state, typename graph::action>;
};

static_assert(MatchRecorderTypes<TicTacToeRecorder>);

template <MatchRecorderTypes Types>
class MatchRecorder {
 public:
	MatchRecorder() = default;
	MatchRecorder(std::vector<typename Types::player> players,
		typename Types::graph::container graph,
		typename Types::graph::rules rules,
		typename Types::storage storage)
			: players_(players), graph_(graph), rules_(rules), storage_(storage) {}

	auto operator()(
		std::stop_token const& token, std::atomic<Recorder::State>& state, tools::MutexQueue<Recorder::Command>& queue)
		-> void {
		while (!token.stop_requested()) {
			auto commands = queue.drain();
			// compact

			// execute
			{
				state = Recorder::State::Recording;
				record_once();
			}
		}
		state = Recorder::State::Stopped;
	}

 private:
	using State = typename Types::graph::state;
	using Action = typename Types::graph::action;

	std::vector<typename Types::player> players_;
	typename Types::graph::container graph_;
	typename Types::graph::rules rules_;
	typename Types::storage storage_;

	std::shared_ptr<spdlog::logger> logger_ = spdlog::default_logger();  // todo: make logger configurable/injectable
	std::mt19937 rng_ = std::mt19937(std::random_device()());
	std::uniform_real_distribution<float> unit_distribution_ = std::uniform_real_distribution<float>(0.0, 1.0);

	auto record_once() -> void {
		// todo: pick randomly?
		State state = graph_.roots().front();

		logger_->debug("match starts");
		std::vector<std::string> player_ids;
		std::ranges::transform(players_, std::back_inserter(player_ids), [&](auto const& player) { return player.id(); });

		Match<State, Action> match{.player_ids = player_ids,
			.start = std::chrono::steady_clock::now(),
			.end = {},
			.plays = {},
			.end_state = {},
			.end_score = Score(0)};

		for (size_t turn = 0; !graph_.is_terminal_at(state); ++turn) {
			auto player = players_[turn % players_.size()];
			Action action = player.choose_play(state, graph_, rules_);
			logger_->debug("turn {}, player {}", turn, player.display_name());

			match.plays.emplace_back(state, action);
			if (!graph_.is_expanded_at(state, action))
				sag::expand(graph_, rules_, state, action);

			state = sag::follow(graph_.edges_at(state, action), UnitValue{unit_distribution_(rng_)});
		}
		match.end = std::chrono::steady_clock::now();
		match.end_state = state;
		match.end_score = rules_.score(state);
		logger_->debug("match ends");

		storage_.add(match, "");

		logger_->debug("match stored");
	}
};

static_assert(std::is_convertible<MatchRecorder<TicTacToeRecorder>, Recorder::Function>());

class MatchRecorder_v0 {
 public:
	MatchRecorder_v0() {
		rand;
		rng_ = std::mt19937(rand());
	}

	template <Graph G>
	auto record_duel(typename G::state root,
		typename G::container& graph,
		typename G::rules const& rules,
		Player<G> auto& first_player,
		Player<G> auto& second_player) -> Match<typename G::state, typename G::action> {
		// validate the root
		auto roots = graph.roots();
		if (std::ranges::find(roots, root) == roots.end())
			return {};

		logger_->debug("match starts");
		Match<typename G::state, typename G::action> match{.player_ids = {first_player.id(), second_player.id()},
			.start = std::chrono::steady_clock::now(),
			.end = {},
			.plays = {},
			.end_state = {},
			.end_score = Score(0)};
		typename G::state state = root;

		for (size_t turn = 0; !graph.is_terminal_at(state); ++turn) {
			logger_->debug("turn {}", turn);
			typename G::action action =
				turn % 2 == 0 ? first_player.choose_play(state, graph, rules) : second_player.choose_play(state, graph, rules);
			match.plays.emplace_back(state, action);
			if (!graph.is_expanded_at(state, action))
				sag::expand(graph, rules, state, action);

			state = sag::follow(graph.edges_at(state, action), UnitValue{unit_distribution_(rng_)});
		}
		match.end = std::chrono::steady_clock::now();
		match.end_state = state;
		match.end_score = rules.score(state);
		logger_->debug("match ends");
		return match;
	}

 private:
	std::shared_ptr<spdlog::logger> logger_ = spdlog::default_logger();
	std::mt19937 rng_;
	std::uniform_real_distribution<float> unit_distribution_ = std::uniform_real_distribution<float>(0.0, 1.0);
};

static_assert(std::semiregular<MatchRecorder_v0>);

}  // namespace sag::rec
