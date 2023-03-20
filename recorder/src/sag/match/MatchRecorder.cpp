#include "MatchRecorder.h"

#include <algorithm>
#include <vector>

#include "sag/match/MatchRecorder.h"

namespace sag::match {

auto MatchRecorder::start() -> void {
	state_ = RecorderState::Recording;
	while (state_ == RecorderState::Recording) {
		do_work_();
	}
}

auto MatchRecorder::reduce_commands_(std::vector<RecorderCommand> const& commands) -> void {}

auto MatchRecorder::drain_inbox_() -> std::vector<RecorderCommand> {
	if (inbox_queue_.empty())
		return {};

	std::vector<RecorderCommand> commands;
	while (true) {
		auto command = inbox_queue_.try_dequeue();
		if (!command.has_value())
			break;

		commands.emplace_back(command.value());
	}
	return commands;
}

}  // namespace sag::match