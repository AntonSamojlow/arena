#pragma once

#include <chrono>
#include <concepts>
#include <cstdint>
#include <string>
#include <tl/expected.hpp>
#include <type_traits>
#include <utility>

#include "sag/match/Match.h"
#include "sag/Failure.h"

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

struct SQLResult {
	std::vector<std::string> header;
	std::vector<std::vector<std::string>> rows;
};

template <typename Connection>
concept SQLConnection = requires(Connection const c_connection) {
													{ c_connection.execute() } -> std::same_as<tl::expected<SQLResult, Failure>>;
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


template <typename S, typename A>
	requires sag::Vertices<S, A>
class SQLMatchStorage {

 public:
	auto add(match::Match<S, A> match, std::string_view extra_data) -> tl::expected<void, Failure>;
}

}// namespace sag::storage
