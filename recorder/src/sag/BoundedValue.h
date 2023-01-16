#pragma once

#include <algorithm>
#include <concepts>

/// <summary>
/// Arithmetic read-only value forced to be between Min and Max.
/// </summary>
template <typename T, int Min, int Max>
	requires std::is_arithmetic_v<T>
struct Boundedvalue {
	Boundedvalue() = default;
	explicit Boundedvalue(T&& initial_value)
			: value_(std::clamp<T>(initial_value, static_cast<T>(Min), static_cast<T>(Max))) {}

	[[nodiscard]] auto value() const -> T { return value_; }

 private:
	T value_ = static_cast<T>(0);
};

static_assert(std::semiregular<Boundedvalue<int, -1, 1>>);
static_assert(std::semiregular<Boundedvalue<double, -1, 1>>);
static_assert(std::semiregular<Boundedvalue<float, 0, 1>>);

using Score = Boundedvalue<float, -1, 1>;
using UnitValue = Boundedvalue<float, 0, 1>;
