#pragma once
#include <stop_token>
#include <thread>

#include "MutexQueue.h"

namespace tools {

class CmdLineThread {
 public:
	CmdLineThread();

	[[nodiscard]] auto queue() -> MutexQueue<std::string>& { return queue_; }

 private:
	MutexQueue<std::string> queue_{};
	std::jthread thread_;
};
}  // namespace tools
