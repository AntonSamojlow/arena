#include <catch2/catch_test_macros.hpp>

#include "tools/UniqueResource.h"

TEST_CASE("UniqueResourceTest", "[tools]") {
	struct WasteBin {
		int count = 0;
		auto increment() -> void { count++; }
	};

	struct TestClass {
		bool alive = true;
	};

	struct Deleter {
		WasteBin* bin = nullptr;
		auto operator()(TestClass& test_class) const {
			test_class.alive = false;
			bin->increment();
		}
	};

	WasteBin waste_bin;
	Deleter const del{&waste_bin};
	{
		tools::unique_resource<TestClass, Deleter> resource{TestClass(), del};
		CHECK(resource.get().alive);

		TestClass const dead_class = {false};
		CHECK(!dead_class.alive);

		// verify a value reset triggers a delete
		CHECK(waste_bin.count == 0);
		resource.reset(dead_class);
		CHECK(waste_bin.count == 1);

		// retrieve new object state
		CHECK(!resource.get().alive);

		// verify an empty reset triggers a delete but disablds 'execute_on_reset'
		CHECK(waste_bin.count == 1);
		resource.reset();
		CHECK(waste_bin.count == 2);
		resource.reset(TestClass{});
		CHECK(waste_bin.count == 2);
	}
	CHECK(waste_bin.count == 3);
}
