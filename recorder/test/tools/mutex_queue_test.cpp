#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <optional>

#include "tools/MutexQueue.h"

TEMPLATE_TEST_CASE("MutexQueue base operations", "[tools]", int, double, std::string) {
	tools::MutexQueue<TestType> queue;

	// prepare two different values
	TestType first{};
	TestType second{};
	if constexpr (std::is_arithmetic_v<TestType>) {
		second = 2;
	}
	if constexpr (std::is_same_v<TestType, std::string>) {
		second = "2";
	}
	CHECK(first != second);

	// test base operations
	CHECK(queue.empty());
	CHECK(queue.size() == 0);  // NOLINT
	queue.push(first);
	queue.emplace(second);

	CHECK(!queue.empty());
	CHECK(queue.size() == 2);

	CHECK(queue.try_dequeue() == first);
	CHECK(queue.size() == 1);
	CHECK(queue.try_dequeue() == second);
	CHECK(queue.empty());
	CHECK(queue.try_dequeue() == std::nullopt);
}
