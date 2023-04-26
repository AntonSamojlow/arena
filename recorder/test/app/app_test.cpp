#include "app/app.h"

#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <sstream>
#include <stop_token>
#include <thread>

using namespace app;

using namespace std::chrono_literals;

TEST_CASE("AppStopTest", "[app]") {
	std::stringstream input{"h\ns\nr\ns\nq\n"};
	App test_app{input};
	int result = test_app.run();
	CHECK(result == 0);
}
