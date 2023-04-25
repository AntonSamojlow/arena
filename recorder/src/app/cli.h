#pragma once

#include <memory>
#include <string>

#include "tools/ThreadHandle.h"

namespace app {
auto cli_thread_loop(std::stop_token const& token, tools::MutexQueue<std::string>* queue) -> void;
}
