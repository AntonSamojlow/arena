#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include <thread>
#include "tools/MutexQueue.h"

using namespace std::chrono_literals;

TEMPLATE_TEST_CASE("MutexQueue base operations", "[tools]", int, double, std::string) {
	tools::MutexQueue<TestType> queue;

	// test base operations
	CHECK(queue.empty());
	CHECK(queue.size() == 0);  // NOLINT
	queue.push(TestType{});

	if constexpr (std::is_arithmetic_v<TestType>) {
		queue.emplace(0);
	} else if constexpr (std::is_same_v<TestType, std::string>) {
		queue.emplace("string");
	}

	CHECK(!queue.empty());
	CHECK(queue.size() == 2);
	SECTION("Test dequeue") {
		CHECK(queue.try_dequeue().has_value());
		CHECK(queue.size() == 1);
		CHECK(queue.try_dequeue().has_value());
		CHECK(queue.empty());
		CHECK(queue.try_dequeue() == std::nullopt);
		CHECK(queue.wait_for_and_dequeue(1ms) == std::nullopt);
	}

	SECTION("Test drain") {
		std::queue<TestType> drained;
		queue.swap(drained);
		CHECK(queue.empty());
		CHECK(drained.size() == 2);
	}
}

struct Ops {
	static auto write(tools::MutexQueue<int>& queue, int count) -> void {
		for (int i = 0; i < count; ++i) {
			queue.push(i);
		}
	}

	static auto read(tools::MutexQueue<int>& queue, int count) -> void {
		for (int i = 0; i < count; ++i) {
			queue.wait_and_dequeue();
		}
	}
};

TEST_CASE("MutexQueue wait operation", "[tools]") {
	tools::MutexQueue<int> queue;

	std::jthread const pusher{[](tools::MutexQueue<int>& target_queue) {
															std::this_thread::sleep_for(100ms);
															target_queue.emplace(1);
															target_queue.push(2);
														},
		std::ref(queue)};

	CHECK(!queue.wait_for_and_dequeue(1ms).has_value());
	CHECK(queue.wait_for_and_dequeue(200ms) == 1);
	CHECK(queue.wait_and_dequeue() == 2);
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
			threads.emplace_back(Ops::write, std::ref(queue), push_count);
		}

		// drain the queue the expected numbers of times
		Ops::read(queue, push_count * thread_count);
		CHECK(queue.empty());
	}

	SECTION("Concurrent reads") {
		// fill the queue
		Ops::write(queue, push_count * thread_count);
		CHECK(queue.size() == static_cast<size_t>(push_count * thread_count));

		// concurrently drain the queue the expected numbers of times
		for (int i = 0; i < thread_count; ++i) {
			threads.emplace_back(Ops::read, std::ref(queue), push_count);
		}

		for (int attempt = 0; attempt < 100; ++attempt) {
			if (queue.empty()) {
				break;
			}
			std::this_thread::sleep_for(10ms);
		}

		CHECK(queue.empty());
	}

	SECTION("Concurrent reads and writes") {
		// concurrently fill the queue
		for (int i = 0; i < thread_count; ++i) {
			threads.emplace_back(Ops::write, std::ref(queue), push_count);
		}

		// concurrently drain the queue the expected numbers of times
		for (int i = 0; i < thread_count; ++i) {
			threads.emplace_back(Ops::read, std::ref(queue), push_count);
		}

		for (int attempt = 0; attempt < 100; ++attempt) {
			if (queue.empty()) {
				break;
			}
			std::this_thread::sleep_for(10ms);
		}

		CHECK(queue.empty());
	}
}
