#pragma once

#include <concepts>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <vector>

namespace tools {

/// Concurrent wrapper around std::queue for multi-consumer, multi-producer scenarios.
template <class T, class Container = std::deque<T>>
class MutexQueue {
 private:
	mutable std::mutex mutex_ = {};
	std::condition_variable condition_var_ = {};
	std::queue<T, Container> queue_ = {};

 public:
	MutexQueue() = default;

	auto push(T const& data) -> void {
		{
			std::lock_guard lock{mutex_};
			queue_.push(data);
		}
		condition_var_.notify_one();
	}

	template <class... Args>
	auto emplace(Args&&... args) -> void {
		{
			std::lock_guard lock{mutex_};
			queue_.emplace(std::forward<Args>(args)...);
		}
		condition_var_.notify_one();
	}

	[[nodiscard]] auto empty() const -> bool {
		std::lock_guard lock{mutex_};
		return queue_.empty();
	}

	[[nodiscard]] auto size() const -> size_t {
		std::lock_guard lock{mutex_};
		return queue_.size();
	}

	auto swap(std::queue<T, Container>& other) noexcept -> void {
		std::lock_guard lock{mutex_};
		queue_.swap(other);
	}

	[[nodiscard]] auto try_dequeue() -> std::optional<T> {
		std::lock_guard lock{mutex_};
		if (queue_.empty()) {
			return std::nullopt;
		}

		T result = queue_.front();
		queue_.pop();
		return result;
	}

	auto wait_and_dequeue() -> T {
		std::unique_lock lock{mutex_};

		condition_var_.wait(lock, [this] { return !queue_.empty(); });
		T result = queue_.front();
		queue_.pop();
		return result;
	}

	template <class Rep, class Period>
	[[nodiscard]] auto wait_for_and_dequeue(const std::chrono::duration<Rep, Period>& time_out) -> std::optional<T> {
		std::unique_lock lock{mutex_};

		if (!condition_var_.wait_for(lock, time_out, [this] { return !queue_.empty(); }))
			return std::nullopt;

		T result = queue_.front();
		queue_.pop();
		return result;
	}
};

}  // namespace tools
