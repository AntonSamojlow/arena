#include <catch2/catch_test_macros.hpp>

#include "tools/MutexQueue.h"
#include "tools/ThreadHandle.h"

using namespace tools;
using namespace std::chrono_literals;

TEST_CASE("ThreadHandleTest", "[tools]") {
	std::queue<int> data;

	SECTION("SingleQueuedThreadHandle") {
		auto producer_loop = [](MutexQueue<int>* outbox) -> void {
			for (int i = 0; i < 3; i++) {
				outbox->emplace(i);
			}
		};
		auto producer_thread = SingleQueuedThreadHandle<int>(producer_loop);
		std::this_thread::sleep_for(1s);
		producer_thread.queue().swap(data);
	}

	SECTION("DoubleQueuedThreadHandle") {
		// NOLINTNEXTLINE(*-swappable-parameters)
		auto forwarder_loop = [](std::stop_token const& token, MutexQueue<int>* inbox, MutexQueue<int>* outbox) -> void {
			while (!token.stop_requested()) {
				if (auto enqueued = inbox->wait_for_and_dequeue(100ms))
					outbox->emplace(enqueued.value());
			}
		};
		auto forwarder_thread = DoubleQueuedThreadHandle<int, int>(forwarder_loop);
		for (int i = 0; i < 3; i++) {
			forwarder_thread.inbox().emplace(i);
		}
		std::this_thread::sleep_for(1s);
		forwarder_thread.outbox().swap(data);
	}

	CHECK(data.size() == 3);
	CHECK(data.front() == 0);
	CHECK(data.back() == 2);
}
