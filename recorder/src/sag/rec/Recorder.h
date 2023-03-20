#pragma once

#include <atomic>
#include <functional>
#include <stop_token>
#include <thread>

#include "sag/GraphConcepts.h"
#include "sag/rec/Match.h"
#include "sag/rec/Player.h"
#include "tools/MutexQueue.h"

namespace sag::rec {

class Recorder {
 public:
	enum class Command { Record, Stop, Quit, Info };
	enum class State { Stopped, Recording, Stopping };
	using Function = std::function<void(std::stop_token const &, std::atomic<State> &, tools::MutexQueue<Command> &)>;
	explicit Recorder(Function function) : thread_(function, std::ref(state_), std::ref(queue_)) {}

	auto request(Command command) -> void;
	auto state() const -> State;

 private:
	tools::MutexQueue<Command> queue_;
	std::atomic<State> state_ = State::Stopped;
	std::jthread thread_;
};

}  // namespace sag::rec
