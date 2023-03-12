#include <vcruntime.h>

#include <atomic>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <optional>
#include <thread>
#include <vector>

#include "tools/MutexQueue.h"

using namespace std::chrono_literals;

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
	CHECK(queue.wait_for_and_dequeue(1ms) == std::nullopt);
}

auto write(tools::MutexQueue<int>& queue, int count) -> void {
	for (int i = 0; i < count; ++i) {
		queue.push(i);
	}
}

auto read(tools::MutexQueue<int>& queue, int count) -> void {
	for (int i = 0; i < count; ++i) {
		auto result = queue.wait_and_dequeue();
		CHECK(result.has_value());
	}
}

TEST_CASE("MutexQueue concurrent operations", "[tools]") {
	tools::MutexQueue<int> queue;

	int const push_count = 100;
	int const thread_count = 100;

	std::vector<std::jthread> threads;
	threads.reserve(thread_count);

	SECTION("Concurrent writes") {
		// concurrently fill the queue
		for (int i = 0; i < thread_count; ++i) {
			threads.emplace_back(::write, std::ref(queue), push_count);
		}

		// drain the queue the expected numbers of times
		read(queue, push_count * thread_count);
		CHECK(queue.empty());
	}

	SECTION("Concurrent reads") {
		// fill the queue
		write(queue, push_count * thread_count);
		CHECK(queue.size() == static_cast<size_t>(push_count * thread_count));

		// concurrently drain the queue the expected numbers of times
		for (int i = 0; i < thread_count; ++i) {
			threads.emplace_back(::read, std::ref(queue), push_count);
		}

		for (int attempt = 0; attempt < 100; ++attempt) {
			if (queue.empty()) {
				break;
			};
			std::this_thread::sleep_for(10ms);
		}

		CHECK(queue.empty());
	}

	SECTION("Concurrent reads and writes") {
		// concurrently fill the queue
		for (int i = 0; i < thread_count; ++i) {
			threads.emplace_back(::write, std::ref(queue), push_count);
		}

		// concurrently drain the queue the expected numbers of times
		for (int i = 0; i < thread_count; ++i) {
			threads.emplace_back(::read, std::ref(queue), push_count);
		}

		for (int attempt = 0; attempt < 100; ++attempt) {
			if (queue.empty()) {
				break;
			};
			std::this_thread::sleep_for(10ms);
		}

		CHECK(queue.empty());
	}
}