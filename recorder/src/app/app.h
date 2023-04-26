#pragma once

#include <iostream>
#include <istream>
namespace app {
class App {
	std::istream& input_source_ = std::cin;

 public:
	App() = default;
	explicit App(std::istream& input_source) : input_source_(input_source) {}

	auto run() -> int;
};

}  // namespace app
