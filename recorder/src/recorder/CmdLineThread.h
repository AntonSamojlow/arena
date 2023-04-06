#pragma once
#include <stop_token>
#include <thread>

#include "recorder/CmdLineThread.h"
#include "tools/MutexQueue.h"
#include "tools/Failure.h"

namespace recorder {

struct CmdLineRequest {
	enum class Type { Quit };
	Type type;
};

class CmdLineThread {
 public:
	CmdLineThread();

	// [[nodiscard]] auto queue() -> tools::MutexQueue<CmdLineRequest>&;

 private:
	tools::Failure failure {};
	// tools::MutexQueue<CmdLineRequest> queue_{};
	std::jthread thread_;
};
}  // namespace recorder
