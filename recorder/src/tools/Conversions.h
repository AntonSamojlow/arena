#pragma once

#include <string_view>
#include <tl/expected.hpp>

#include "Failure.h"

namespace tools {

enum class ConversionError { Unknown = 0, OutOfRange = 1, Impossible = 2 };

auto to_int64(std::string_view text) -> tl::expected<int64_t, Failure>;
}  // namespace tools
