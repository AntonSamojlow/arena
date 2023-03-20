#pragma once

#include <vector>

#include "sag/GraphConcepts.h"
#include "sag/match/Match.h"
#include "sag/match/Player.h"
#include "tools/MutexQueue.h"

namespace sag::match {

enum class RecorderCommand { Record, Stop, Quit, Info };

enum class RecorderState { Stopped, Recording, Stopping };

class MatchRecorder {
 public:
	auto start() -> void;

 private:
	auto do_work_() -> void{};

	auto reduce_commands_(std::vector<RecorderCommand> const& commands) -> void;
	auto drain_inbox_() -> std::vector<RecorderCommand>;
	tools::MutexQueue<RecorderCommand> inbox_queue_;
	RecorderState state_ = RecorderState::Stopped;
};

}  // namespace sag::match