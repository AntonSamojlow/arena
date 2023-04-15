#pragma once
#include <spdlog/spdlog.h>

#include <queue>
#include <string>

#include "Match.h"
#include "tools/MutexQueue.h"
#include "tools/ThreadHandle.h"

namespace sag::rec {

enum class Signal { Record, Halt, Quit, Status };

template <MatchRecorderTypes Types>
class MatchRecorder {
 public:
	MatchRecorder(std::vector<typename Types::player>&& players,
		typename Types::graph::container&& graph,
		typename Types::graph::rules&& rules,
		typename Types::storage&& storage)
			: players_(std::move(players)),
				graph_(std::move(graph)),
				rules_(std::move(rules)),
				storage_(std::move(storage)) {}

	// recorder is callable: it may run in a thread, with a queue for control signals
	auto operator()(std::stop_token const& token, tools::MutexQueue<Signal>* queue) -> void {
		logger_->info("recorder thread start");
		while (!token.stop_requested()) {
			if (is_running_)
				record_once();

			while (auto signal = queue->try_dequeue()) {
				switch (signal.value()) {
					case Signal::Record:
						logger_->info("record signal");
						is_running_ = true;
						break;
					case Signal::Halt:
						logger_->info("halt signal");
						is_running_ = false;
						break;
					case Signal::Quit: logger_->info("quit signal"); return;
					case Signal::Status:
						logger_->info("status signal");
						generate_info();
						break;
				}
			}
		}
		logger_->info("recorder thread end");
	}

 private:
	bool is_running_ = false;
	std::vector<typename Types::player> players_;
	typename Types::graph::container graph_;
	typename Types::graph::rules rules_;
	typename Types::storage storage_;

	std::shared_ptr<spdlog::logger> logger_ = spdlog::default_logger();  // todo: make logger configurable/injectable
	std::mt19937 rng_ = std::mt19937(std::random_device()());
	std::uniform_real_distribution<float> unit_distribution_ = std::uniform_real_distribution<float>(0.0, 1.0);

	auto generate_info() const -> void {
		logger_->info("match recorder status: {} players, running={}", players_.size(), is_running_);
	}

	auto record_once() -> void {
		using State = typename Types::graph::state;
		using Action = typename Types::graph::action;

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
			.end_score = tools::Score(0)};

		for (size_t turn = 0; !graph_.is_terminal_at(state); ++turn) {
			auto player = players_[turn % players_.size()];
			Action action = player.choose_play(state, graph_, rules_);
			logger_->debug("turn {}, player {}", turn, player.display_name());

			match.plays.emplace_back(state, action);
			if (!graph_.is_expanded_at(state, action))
				sag::expand(graph_, rules_, state, action);

			state = sag::follow(graph_.edges_at(state, action), tools::UnitValue{unit_distribution_(rng_)});
		}
		match.end = std::chrono::steady_clock::now();
		match.end_state = state;
		match.end_score = rules_.score(state);
		logger_->debug("match ends");

		storage_.add(match, "");

		logger_->debug("match stored");
	}
};

/// Handle to a recorder thread with input queue for control signals
template <MatchRecorderTypes Types>
struct RecorderThreadHandle : tools::SingleQueuedThreadHandle<sag::rec::Signal> {
 public:
	explicit RecorderThreadHandle(MatchRecorder<Types>&& recorder)
			: tools::SingleQueuedThreadHandle<sag::rec::Signal>(std::move(recorder)) {}
};

}  // namespace sag::rec
