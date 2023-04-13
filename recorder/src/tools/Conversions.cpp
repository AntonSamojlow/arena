#include "Conversions.h"

#include <tl/expected.hpp>

#include "tools/Failure.h"

namespace tools {
auto to_int64(std::string_view text) -> tl::expected<int64_t, Failure> {
	// [based on https://en.cppreference.com/w/c/string/byte/strtol]
	// errno can be set to any non-zero value by a library function call
	// regardless of whether there was an error, so it needs to be cleared
	// in order to check the error set by strtol
	errno = 0;
	char *end;  // NOLINT strtoll check logic requires non-nullptr
	const int64_t result = strtoll(text.data(), &end, 10);
	if (text.data() == end) {
		return tl::unexpected<Failure>{
			{.code = static_cast<int>(ConversionError::Impossible), .reason = "strtol conversion not possible"}};
	}
	if (errno == ERANGE) {
		return tl::unexpected<Failure>{{.code = static_cast<int>(ConversionError::OutOfRange),
			.reason = "strtol out of range, value: " + std::to_string(result)}};
	}
	return result;
}
}  // namespace tools
