#pragma once

#include <bitset>

#include "sag/match/Match.h"
#include "tools/Conversions.h"
#include "tools/SQLiteConnection.h"

namespace sag::storage {

// clang-format off
template <typename T>
concept SqlTypeConverter = requires(typename T::original value) {
	// return string should be a valid value when used in an sql INSERT statement
	{ T::to_insert_text(value) } -> std::same_as<std::string>;
	// return string should be a valid column type for sql CREATE TABLE statements
	{ T::sql_type() } -> std::same_as<std::string>;
};
// clang-format on

struct TextConverter {
	using original = std::string;
	auto static to_insert_text(original const& value) -> std::string { return "'" + value + "'"; }
	auto static sql_type() -> std::string { return "text"; }
};
static_assert(SqlTypeConverter<TextConverter>);

template <size_t N>
struct BitsetConverter {
	using original = std::bitset<N>;
	auto static to_insert_text(original const& value) -> std::string { return "'" + value.to_string() + "'"; }
	auto static sql_type() -> std::string { return "blob"; }
};
static_assert(SqlTypeConverter<BitsetConverter<2>>);

template <class T>
	requires std::is_integral_v<T>
struct IntegerConverter {
	using original = T;
	auto static to_insert_text(original value) -> std::string { return std::to_string(value); }
	auto static sql_type() -> std::string { return "integer"; }
};
static_assert(SqlTypeConverter<IntegerConverter<int>>);
static_assert(SqlTypeConverter<IntegerConverter<int64_t>>);

template <class T>
	requires std::is_floating_point_v<T>
struct FloatingPointConverter {
	using original = T;
	auto static to_insert_text(original value) -> std::string { return std::to_string(value); }
	auto static sql_type() -> std::string { return "real"; }
};
static_assert(SqlTypeConverter<FloatingPointConverter<float>>);
static_assert(SqlTypeConverter<FloatingPointConverter<double>>);

template <class Duration>
struct TimeConverter {
	using original = std::chrono::time_point<Duration>;
	auto static to_insert_text(original const& value) -> std::string {
		return std::to_string(value.time_since_epoch().count());
	}
	auto static sql_type() -> std::string { return "integer"; }
};
static_assert(SqlTypeConverter<TimeConverter<std::chrono::microseconds>>);
static_assert(SqlTypeConverter<TimeConverter<std::chrono::days>>);
static_assert(SqlTypeConverter<TimeConverter<std::chrono::steady_clock>>);

template <SqlTypeConverter SqlState, SqlTypeConverter SqlAction>
	requires sag::Vertices<typename SqlState::original, typename SqlAction::original>
class SQLiteMatchStorage {
	using S = typename SqlState::original;
	using A = typename SqlAction::original;

 public:
	explicit SQLiteMatchStorage(std::unique_ptr<tools::SQLiteConnection>&& connection)
			: connection_(std::move(connection)) {
		auto result = initialize_tables();
		if (!result.has_value())
			throw std::runtime_error(result.error().reason);
	}

	auto add(match::Match<S, A> match, std::string_view extra_data) const -> tl::expected<void, tools::Failure> {
		std::string command =
			fmt::format("INSERT INTO matches (score, starttime, endtime, extra_data) VALUES ({}, {}, {}, {}) RETURNING id;",
				FloatingPointConverter<float>::to_insert_text(match.end_score.value()),
				TimeConverter<std::chrono::steady_clock>::to_insert_text(match.start),
				TimeConverter<std::chrono::steady_clock>::to_insert_text(match.end),
				TextConverter::to_insert_text(std::string(extra_data)));
		auto result = connection_->execute(command);
		if (!result.has_value())
			return tl::unexpected<tools::Failure>(result.error());
		int64_t match_id = std::atoi(result->rows.front()[0].c_str());
		command = "INSERT INTO records (match_id, turn_nr, state, action) VALUES";
		for (size_t turn_nr = 0; turn_nr < match.plays.size(); ++turn_nr) {
			sag::match::Play<S, A> const& play = match.plays[turn_nr];
			command += fmt::format("({}, {}, {}, {}),",
				IntegerConverter<int64_t>::to_insert_text(match_id),
				IntegerConverter<size_t>::to_insert_text(turn_nr),
				SqlState::to_insert_text(play.state()),
				SqlAction::to_insert_text(play.action()));
		}
		// replace last comma by a semicolon
		command[command.size() - 1] = ';';
		result = connection_->execute(command);
		if (!result.has_value())
			return tl::unexpected<tools::Failure>(result.error());

		return {};
	}

	[[nodiscard]] auto count_records() const -> tl::expected<size_t, tools::Failure> { return count_rows("records"); }
	[[nodiscard]] auto count_matches() const -> tl::expected<size_t, tools::Failure> { return count_rows("matches"); }

 private:
	std::unique_ptr<tools::SQLiteConnection> connection_;

	[[nodiscard]] auto count_rows(std::string_view table_name) const -> tl::expected<size_t, tools::Failure> {
		std::string const check_table = fmt::format("SELECT COUNT(*) name FROM {};", table_name);
		auto result = connection_->execute(check_table);
		if (!result.has_value())
			return tl::unexpected<tools::Failure>{result.error()};
		auto converted = tools::to_int64(result.value().rows.front()[0]);
		if (!converted.has_value())
			return tl::unexpected<tools::Failure>{converted.error()};
		return converted.value();
	}

	[[nodiscard]] auto tables_exist(std::string_view table_name) const -> bool {
		std::string const check_table =
			fmt::format("SELECT name FROM sqlite_master WHERE type='table' AND name='{}';", table_name);
		return connection_->execute(check_table).has_value();
	}

	/// Attempts to initialize the tables. Does not return a failure if the tables exist.
	[[nodiscard]] auto initialize_tables() const -> tl::expected<void, tools::Failure> {
		std::string const create_matches =
			"CREATE TABLE matches(id integer, score real, starttime integer, endtime integer, extra_data text,"
			"PRIMARY KEY(id));";

		std::string const create_records = fmt::format(
			"CREATE TABLE records(match_id integer, turn_nr integer, state {}, action {},"
			"PRIMARY KEY(match_id, turn_nr), FOREIGN KEY(match_id) REFERENCES matches(id));",
			SqlState::sql_type(),
			SqlAction::sql_type());

		auto result = connection_->execute(create_matches);
		if (!result.has_value() && !tables_exist("matches"))
			return tl::unexpected<tools::Failure>{result.error()};

		result = connection_->execute(create_records);
		if (!result.has_value() && !tables_exist("records"))
			return tl::unexpected<tools::Failure>{result.error()};

		return {};
	}
};

}  // namespace sag::storage
