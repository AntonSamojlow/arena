#include <catch2/catch_test_macros.hpp>

#include "tools/BoundedValue.h"

TEST_CASE("UnitValue test", "[tools]") {
	tools::UnitValue const default_value(0.0F);
	tools::UnitValue const zero(0.0F);
	tools::UnitValue const half(0.5F);
	tools::UnitValue const one(1.0F);
	tools::UnitValue const excess(10.0F);
	tools::UnitValue const negative(-1.0F);

	CHECK(default_value.value() == 0.0F);
	CHECK(zero.value() == 0.0F);
	CHECK(half.value() == 0.5F);
	CHECK(one.value() == 1.0F);
	CHECK(excess.value() == 1.0F);
	CHECK(negative.value() == 0.0F);
	CHECK(default_value == zero);
	CHECK(default_value < half);
	CHECK(half < one);
	CHECK(excess == one);
	CHECK(negative == zero);
	CHECK(excess > negative);
}
