#pragma once
#include <stop_token>
#include <thread>

#include "tools/MutexQueue.h"

namespace recorder {

struct CmdLineRequest {
	enum class Type { Quit };
	Type type;
};

class CmdLineThread {
 public:
	CmdLineThread();

	[[nodiscard]] auto queue() -> tools::MutexQueue<CmdLineRequest>& {return queue_;}

 private:
	tools::MutexQueue<CmdLineRequest> queue_{};
	std::jthread thread_;
};
}  // namespace recorder
