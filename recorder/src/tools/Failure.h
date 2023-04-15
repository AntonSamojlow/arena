#pragma once

#include <concepts>
#include <string>
namespace tools {

struct Failure {
	int code;
	std::string reason;

	friend auto operator<=>(const Failure&, const Failure&) = default;
};

static_assert(std::regular<Failure>);
}  // namespace tools
