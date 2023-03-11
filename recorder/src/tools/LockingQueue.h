#pragma once

#include <vcruntime.h>

#include <queue>


template <typename T>
class LockingQueue {
	std::queue<T> queue_;

 public:
	// Element access

	auto front() const -> T const& {}
	auto front() -> T& {}

	auto back() const -> T const& {}
	auto back() -> T& {}

	auto empty() -> bool {}
	auto size() -> size_t {}

	// push
	// inserts element at the end

	// emplace
	// (C++11)
	// constructs element in-place at the end

	// pop
	// removes the first element

	// swap
	// swaps the contents
};
