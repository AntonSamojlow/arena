#pragma once
#include <spdlog/spdlog.h>

#include <exception>
#include <memory>
#include <queue>
#include <random>
#include <string>

#include "Match.h"
#include "sag/GraphConcepts.h"
#include "sag/match/Player.h"
#include "tools/MutexQueue.h"
#include "tools/ThreadHandle.h"

namespace sag::match {

enum class Signal { Record, Halt, Quit, Status };

template <Graph G, Storage<typename G::state, typename G::action> S>
class MatchRecorder {
 public:
	MatchRecorder(std::vector<std::unique_ptr<Player<G>>>&& players,
		typename G::container&& graph,
		typename G::rules&& rules,
		S&& storage,
		spdlog::logger&& logger)
			: players_(std::move(players)),
				graph_(std::move(graph)),
				rules_(std::move(rules)),
				storage_(std::move(storage)),
				logger_(std::move(logger)) {}

	// recorder is callable: it may run in a thread, with a queue for control signals
	auto operator()(std::stop_token const& token, tools::MutexQueue<Signal>* queue) -> void {
		try {
			logger_.info("recorder thread start");
			while (!token.stop_requested()) {
				if (is_running_)
					record_once();

				while (auto signal = queue->try_dequeue()) {
					switch (signal.value()) {
						case Signal::Record:
							logger_.info("record signal");
							is_running_ = true;
							break;
						case Signal::Halt:
							logger_.info("halt signal");
							is_running_ = false;
							break;
						case Signal::Quit: logger_.info("quit signal"); return;
						case Signal::Status:
							logger_.info("status signal");
							generate_info();
							break;
					}
				}
			}
			logger_.info("recorder thread end");
		} catch (std::exception const& exc) {
			logger_.error("match recorder exception: {}", exc.what());
		}
	}

 private:
	bool is_running_ = false;
	std::vector<std::unique_ptr<Player<G>>> players_;
	typename G::container graph_;
	typename G::rules rules_;
	S storage_;

	mutable spdlog::logger logger_;
	std::mt19937 rng_ = std::mt19937(std::random_device()());
	std::uniform_real_distribution<float> unit_distribution_ = std::uniform_real_distribution<float>(0.0, 1.0);

	auto generate_info() const -> void {
		std::string player_names = std::string{players_.front()->display_name()};
		for (size_t i = 1; i < players_.size(); ++i)
			player_names = std::move(player_names) + ", " + std::string{players_[i]->display_name()};

		logger_.info("match recorder status: {} players ({}), running={}", players_.size(), player_names, is_running_);
	}

	auto record_once() -> void {
		using State = typename G::state;
		using Action = typename G::action;

		// todo: pick randomly?
		State state = graph_.roots().front();

		logger_.debug("match starts");
		std::vector<std::string> player_ids;
		std::ranges::transform(
			players_, std::back_inserter(player_ids), [&](auto const& player) { return std::string{player->id()}; });

		Match<State, Action> match{.player_ids = player_ids,
			.start = std::chrono::steady_clock::now(),
			.end = {},
			.plays = {},
			.end_state = {},
			.end_score = tools::Score(0)};

		for (size_t turn = 0; !graph_.is_terminal_at(state); ++turn) {
			Player<G>* player = players_[turn % players_.size()].get();
			logger_.debug("turn {}, player {}...", turn, player->display_name());
			Action action = player->choose_play(state, graph_, rules_);

			match.plays.emplace_back(state, action);
			if (!graph_.is_expanded_at(state, action))
				sag::expand(graph_, rules_, state, action);

			state = sag::follow(graph_.edges_at(state, action), tools::UnitValue{unit_distribution_(rng_)});

			if constexpr (sag::CountingGraphContainer<typename G::container, typename G::state, typename G::action>) {
				logger_.debug("clearing and rerooting graph with {:L} states, {:L} actions and {:L} edges ...",
					graph_.state_count(),
					graph_.action_count(),
					graph_.edge_count());
			} else {
				logger_.debug("clearing and rerooting graph ...");
			}
			graph_.clear_and_reroot({state});
			logger_.debug("graph cleared");
		}
		match.end = std::chrono::steady_clock::now();
		match.end_state = state;
		match.end_score = rules_.score(state);
		logger_.debug("match ends");

		storage_.add(match, "");
		logger_.info("match stored");
	}
};

/// Handle to a recorder thread with input queue for control signals
template <class Rec>
struct RecorderThreadHandle : tools::SingleQueuedThreadHandle<sag::match::Signal> {
 public:
	explicit RecorderThreadHandle(Rec&& recorder)
			: tools::SingleQueuedThreadHandle<sag::match::Signal>(std::move(recorder)) {}
};

}  // namespace sag::match
