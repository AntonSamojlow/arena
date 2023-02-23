#pragma once

#include <fmt/core.h>

#include <chrono>
#include <concepts>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tl/expected.hpp>
#include <type_traits>
#include <utility>

#include "sag/Failure.h"
#include "sag/match/Match.h"

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
concept SQLConnection = requires(Connection const c_connection, std::string_view statement) {
													{ c_connection.execute(statement) } -> std::same_as<tl::expected<SQLResult, Failure>>;
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


template <SqlInsertable S, SqlInsertable A, SQLConnection C>
	requires sag::Vertices<S, A>
class SQLMatchStorage {
 public:
	explicit SQLMatchStorage(std::unique_ptr<C>&& connection) : connection_(std::move(connection)) {
		auto result = initialize_tables();
		if (!result.has_value())
			throw std::runtime_error(result.error().reason);
	}

	auto add(Match<S, A> match, std::string_view extra_data) const -> tl::expected<void, Failure> {
		std::string command =
			fmt::format("INSERT INTO matches (score, starttime, endtime, extra_data) VALUES ({}, {}, {}, {}) RETURNING id",
				match.end_score.sql_insert_text(),
				match.start_epoch.sql_insert_text(),
				match.end_epoch.sql_insert_text(),
				"'NO_EXTRA_DATA'");
			connection_->execute() [TODOOOO]


	}

 private:
	std::string state_db_type = "integer";
	std::string action_db_type = "integer";
	std::unique_ptr<C> connection_;
	[[nodiscard]] auto initialize_tables() const -> tl::expected<void, Failure> {
		std::string const create_matches =
			"CREATE TABLE matches(id integer, score real, starttime integer, endtime integer, extra_data text,"
			"PRIMARY KEY(id));";

		std::string const create_records = fmt::format(
			"CREATE TABLE records(match_id integer, turn_nr integer, state {}, action {},"
			"PRIMARY KEY(match_id, turn_nr), FOREIGN KEY(match_id) REFERENCES matches(id));",
			state_db_type,
			action_db_type);

		auto result = connection_->execute(create_matches);
		if (!result.has_value())
			return tl::unexpected<Failure>{result.error()};

		result = connection_->execute(create_records);
		if (!result.has_value())
			return tl::unexpected<Failure>{result.error()};

		return {};
	}
};

}  // namespace sag::storage
