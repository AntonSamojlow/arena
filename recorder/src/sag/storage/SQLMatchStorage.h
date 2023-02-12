#pragma once

#include <chrono>
#include <concepts>
#include <cstdint>
#include <string>
#include <tl/expected.hpp>
#include <type_traits>
#include <utility>

#include "Failure.h"
#include "match/Match.h"

namespace sag::storage {

template <typename T>
concept SqlInsertable = requires(T type) {
													// return string should be a valid value when used in an sql INSERT statement
													{ type.sql_insert_text() } -> std::same_as<std::string>;
												};

struct StringSqlValue {
	std::string value;
	[[nodiscard]] auto sql_insert_text() const -> std::string { return "'" + value + "'"; }
};

template <typename T>
	requires std::is_arithmetic_v<T>
struct NumericSqlValue {
	T value;
	[[nodiscard]] auto sql_insert_text() const -> std::string { return std::to_string(value); }
};

template <typename H>
concept SQLHandler = requires(H const c_handler) {
											 { c_handler.open() } -> std::same_as<tl::expected<void, Failure>>;
										 };

template <SqlInsertable S, SqlInsertable A>
struct Match {
	std::vector<StringSqlValue> player_ids;
	NumericSqlValue<uint64_t> start_epoch{0};
	NumericSqlValue<uint64_t> end_epoch{0};
	std::vector<sag::match::Play<S, A>> plays;
	S end_state;
	NumericSqlValue<float> end_score{0.0};
};

}  // namespace sag::storage
