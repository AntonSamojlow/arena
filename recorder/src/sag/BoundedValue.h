#pragma once

#include <algorithm>
#include <concepts>

namespace sag {

/// <summary>
/// Arithmetic read-only value forced to be between Min and Max.
/// </summary>
template <typename T, int Min, int Max>
	requires std::is_arithmetic_v<T>
struct Boundedvalue {
	Boundedvalue() = default;
	explicit Boundedvalue(T const& initial_value)
			: value_(std::clamp<T>(initial_value, static_cast<T>(Min), static_cast<T>(Max))) {}
	explicit Boundedvalue(T&& initial_value)
			: value_(std::clamp<T>(std::move(initial_value), static_cast<T>(Min), static_cast<T>(Max))) {}

	[[nodiscard]] auto value() const -> T { return value_; }

#pragma GCC diagnostic push
// reason: https://github.com/llvm/llvm-project/issues/55919
#pragma GCC diagnostic ignored "-Wfloat-equal"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"

	friend auto operator<=>(const Boundedvalue&, const Boundedvalue&) = default;

#pragma GCC diagnostic pop

 private:
	T value_ = static_cast<T>(0);
};

using Score = Boundedvalue<float, -1, 1>;
using UnitValue = Boundedvalue<float, 0, 1>;
using NonNegative = Boundedvalue<float, 0, std::numeric_limits<int>::max()>;

static_assert(std::regular<Boundedvalue<int, -1, 1>>);
static_assert(std::regular<Boundedvalue<double, -1, 1>>);
static_assert(std::regular<Score>);
static_assert(std::regular<UnitValue>);
static_assert(std::regular<NonNegative>);

}  // namespace sag
