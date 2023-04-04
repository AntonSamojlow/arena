#pragma once
#include "MatchConcepts.h"
#include "tools/MutexQueue.h"
#include "sag/TicTacToe.h"
#include "sag/storage/MemoryMatchStorage.h"

namespace sag::rec {

struct TicTacToeRecorder {
	using graph = tic_tac_toe::Graph;
	using player = RandomPlayer<tic_tac_toe::Graph>;
	using storage = storage::MemoryMatchStorage<typename graph::state, typename graph::action>;
};

static_assert(MatchRecorderTypes<TicTacToeRecorder>);

template <MatchRecorderTypes Types>
class MatchRecorder {
	enum class Signal { Record, Stop, Quit };

 public:
	MatchRecorder() = default;
	MatchRecorder(std::vector<typename Types::player> players,
		typename Types::graph::container graph,
		typename Types::graph::rules rules,
		typename Types::storage storage)
			: players_(players), graph_(graph), rules_(rules), storage_(storage) {}

	auto operator()(
		std::stop_token const& token, std::atomic<bool>& is_running, tools::MutexQueue<Signal>& queue)
		-> void {
		is_running = false;
		while (!token.stop_requested()) {
			if (is_running)
				record_once();

			// check and parse accumulated queue content
			if (parse_queue_and_check_info(is_running, queue))
				generate_info();

			if (!is_running) {
				// wait for next queue content
				switch (queue.wait_and_dequeue()) {
					// collapse record and stop to the last one wins:
					case Signal::Record: is_running = false; break;
					case Signal::Stop: break;
					case Signal::Quit: return;
				}
			}
		}
	}

 private:


	std::vector<typename Types::player> players_;
	typename Types::graph::container graph_;
	typename Types::graph::rules rules_;
	typename Types::storage storage_;

	tools::MutexQueue<Signal> signal_queue_;
	std::jthread thread_;

	std::shared_ptr<spdlog::logger> logger_ = spdlog::default_logger();  // todo: make logger configurable/injectable
	std::mt19937 rng_ = std::mt19937(std::random_device()());
	std::uniform_real_distribution<float> unit_distribution_ = std::uniform_real_distribution<float>(0.0, 1.0);

	auto generate_info() const -> void { logger_->debug("{} players", players_.size()); }

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

	/// Drains and parses the accumulated queue content, collapsing repeated state changes into one change.
	/// Returns true if info output was requested; false otherwise.
	auto parse_queue_and_check_info(std::atomic<bool>& is_running, tools::MutexQueue<Signal>& queue)
		-> bool {
		auto commands = queue.drain();

		if (commands.empty())

		std::optional<Recorder::Signal> last_state_change_command = std::nullopt;
		while (!commands.empty()) {
			switch (commands.front()) {
				case Signal::Record: last_state_change_command = Signal::Record; break;
				case Signal::Stop: last_state_change_command = Signal::Stop; break;
				case Signal::Quit: is_running = false; return info_requested;
			}
			commands.pop();
		}
		// collapse record and stop commands (last one wins)
		if (last_state_change_command.has_value()) {
			if (last_state_change_command == Signal::Stop)
				state = Recorder::State::Stopped;
			if (last_state_change_command == Signal::Record)
				state = Recorder::State::Recording;
		}

		return info_requested;
	}
};

}  // namespace sag::rec
