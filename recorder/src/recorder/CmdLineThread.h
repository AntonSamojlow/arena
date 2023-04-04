#pragma once

#include <stop_token>
#include <thread>

#include "tools/MutexQueue.h"

namespace recorder {

struct CmdLineRequest {};

class CmdLineThread {
 public:
	CmdLineThread() = default;

	[[nodiscard]] auto queue() -> tools::MutexQueue<CmdLineRequest>&;

 private:
	auto thread_loop(std::stop_token& token) -> void;
	tools::MutexQueue<CmdLineRequest> queue_;
	std::jthread thread_;
};
}  // namespace recorder
