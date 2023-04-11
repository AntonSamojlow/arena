#include "tools/Conversions.h"

#include <catch2/catch_test_macros.hpp>
#include <limits>
#include <string>

TEST_CASE("Conversion test", "[tools]") {
	auto conversion = tools::to_int64("abc");
	CHECK(!conversion.has_value());
	CHECK(conversion.error().code == static_cast<int>(tools::ConversionError::Impossible));

	conversion = tools::to_int64("9223372036854775808");
	CHECK(!conversion.has_value());
	CHECK(conversion.error().code == static_cast<int>(tools::ConversionError::OutOfRange));

	conversion = tools::to_int64("-9223372036854775809");
	CHECK(!conversion.has_value());
	CHECK(conversion.error().code == static_cast<int>(tools::ConversionError::OutOfRange));

	conversion = tools::to_int64("9223372036854775807");
	CHECK(conversion.has_value());
	CHECK(conversion.value() == 9223372036854775807I64);

	conversion = tools::to_int64("-9223372036854775808");
	CHECK(conversion.has_value());
	CHECK(conversion.value() == (-9223372036854775807I64 - 1));

	conversion = tools::to_int64("0");
	CHECK(conversion.has_value());
	CHECK(conversion.value() == 0);

	conversion = tools::to_int64("-9.345");
	CHECK(conversion.has_value());
	CHECK(conversion.value() == -9);
}
