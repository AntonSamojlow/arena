#include "app/app.h"

#include <catch2/catch_test_macros.hpp>

#include "../helpers.h"
#include "app/config.h"

using namespace app;

using namespace std::chrono_literals;

TEST_CASE("AppStopTest", "[app]") {
	std::stringstream input{"h\ns\nr\ns\nq\n"};
	App test_app{input};
	auto config = App::create_example_config();
	const int result = test_app.run(config);
	CHECK(result == 0);
}
