#pragma once

#include <iostream>
#include <istream>

#include "config.h"

namespace app {
class App {
	std::istream& input_source_ = std::cin;

 public:
	App() = default;
	explicit App(std::istream& input_source) : input_source_(input_source) {}

	auto run(config::Recorder const& config) -> int;
};

}  // namespace app
