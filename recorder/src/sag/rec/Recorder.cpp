#include "Recorder.h"

#include "sag/rec/Recorder.h"

namespace sag::rec {
auto Recorder::request(Recorder::Command command) -> void {
	// for quit-commands, also cancel the threads token
	if (command == Recorder::Command::Quit)
		thread_.get_stop_source().request_stop();

	queue_.emplace(command);
}

auto Recorder::state() const -> Recorder::State {
	return state_;
}

}  // namespace sag::rec
