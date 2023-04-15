#pragma once

#include <thread>

#include "MutexQueue.h"
#include "tools/MutexQueue.h"

namespace tools {

template <class Function, class... QueueData>
concept ThreadFunction = std::invocable<Function, MutexQueue<QueueData> *...> ||
												 std::invocable<Function, std::stop_token const &, MutexQueue<QueueData> *...>;

template <class QueueData>
class SingleQueuedThreadHandle {
 public:
	template <class Function>
		requires ThreadFunction<Function, QueueData> &&
						 (!std::same_as<SingleQueuedThreadHandle, std::remove_cvref_t<Function>>)
	explicit SingleQueuedThreadHandle(Function &&function)  // NOLINT(bugprone-forwarding-reference-overload)
			: thread_(std::forward<Function>(function), queue_.get()) {}

	~SingleQueuedThreadHandle() = default;

	// no copy, but allow move
	SingleQueuedThreadHandle(const SingleQueuedThreadHandle &) = delete;
	auto operator=(const SingleQueuedThreadHandle &) -> SingleQueuedThreadHandle & = delete;
	SingleQueuedThreadHandle(SingleQueuedThreadHandle &&other) noexcept
			: queue_(std::move(other.queue_)), thread_(std::move(other.thread_)) {}
	auto operator=(SingleQueuedThreadHandle &&other) noexcept -> SingleQueuedThreadHandle & {
		thread_ = std::move(other.thread_);
		queue_ = std::move(other.queue_);
	}

	[[nodiscard]] auto queue() -> MutexQueue<QueueData> & { return *queue_; }

 private:
	std::unique_ptr<MutexQueue<QueueData>> queue_ = std::make_unique<MutexQueue<QueueData>>();
	std::jthread thread_;
};

template <class InputData, class OutputData>
class DoubleQueuedThreadHandle {
 public:
	template <class Function>
		requires ThreadFunction<Function, InputData, OutputData> &&
						 (!std::same_as<DoubleQueuedThreadHandle, std::remove_cvref_t<Function>>)
	explicit DoubleQueuedThreadHandle(Function &&function)  // NOLINT(bugprone-forwarding-reference-overload)
			: thread_(std::forward<Function>(function), inbox_.get(), outbox_.get()) {}

	~DoubleQueuedThreadHandle() = default;

	// no copy, but allow move
	DoubleQueuedThreadHandle(const DoubleQueuedThreadHandle &) = delete;
	auto operator=(const DoubleQueuedThreadHandle &) -> DoubleQueuedThreadHandle & = delete;
	DoubleQueuedThreadHandle(DoubleQueuedThreadHandle &&other) noexcept
			: outbox_(std::move(outbox_)), inbox_(std::move(inbox_)), thread_(std::move(other.thread_)) {}
	auto operator=(DoubleQueuedThreadHandle &&other) noexcept -> DoubleQueuedThreadHandle & {
		thread_ = std::move(other.thread_);
		outbox_ = std::move(other.outbox_);
		inbox_ = std::move(other.inbox_);
	}

	[[nodiscard]] auto inbox() -> MutexQueue<InputData> & { return *inbox_; }
	[[nodiscard]] auto outbox() -> MutexQueue<OutputData> & { return *outbox_; }

 private:
	std::unique_ptr<MutexQueue<OutputData>> outbox_ = std::make_unique<MutexQueue<OutputData>>();
	std::unique_ptr<MutexQueue<InputData>> inbox_ = std::make_unique<MutexQueue<InputData>>();
	std::jthread thread_;
};

static_assert(!std::is_copy_assignable_v<SingleQueuedThreadHandle<int>>);
static_assert(!std::is_copy_constructible_v<SingleQueuedThreadHandle<int>>);
static_assert(std::is_move_assignable_v<SingleQueuedThreadHandle<int>>);
static_assert(std::is_move_constructible_v<SingleQueuedThreadHandle<int>>);

static_assert(!std::is_copy_assignable_v<DoubleQueuedThreadHandle<std::string, int>>);
static_assert(!std::is_copy_constructible_v<DoubleQueuedThreadHandle<std::string, int>>);
static_assert(std::is_move_assignable_v<DoubleQueuedThreadHandle<std::string, int>>);
static_assert(std::is_move_constructible_v<DoubleQueuedThreadHandle<std::string, int>>);

}  // namespace tools
