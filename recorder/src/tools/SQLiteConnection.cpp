#include "SQLiteConnection.h"

#include <fmt/core.h>
#include <spdlog/logger.h>
#include <sqlite3.h>

#include <exception>
#include <memory>
#include <string>
#include <tl/expected.hpp>
#include <utility>

#include "Failure.h"

namespace {

/// callback that both reads rows and logs
auto read_rows_callback(void* input_ptr, int argc, char** argv, char** azColName) -> int {  // NOLINT (sqlite has C-API)
	tools::SQLResult& result = *static_cast<tools::SQLResult*>(input_ptr);

	if (result.header.empty()) {
		result.header.reserve(static_cast<size_t>(argc));
		for (int i = 0; i < argc; i++) {
			result.header.emplace_back(azColName[i]);  // NOLINT (sqlite has C-API)
		}
	}

	std::vector<std::string> row;
	row.reserve(static_cast<size_t>(argc));
	for (int i = 0; i < argc; i++) {
		row.emplace_back(argv[i]);  // NOLINT (sqlite has C-API)
	}
	result.rows.push_back(row);
	return 0;
}

}  // namespace

namespace tools {

SQLiteConnection::SQLiteConnection(std::string_view file_path, bool open_read_only) {
	initialize(file_path, open_read_only);
}

SQLiteConnection::SQLiteConnection(
	std::string_view file_path, bool open_read_only, std::shared_ptr<spdlog::logger> logger)
		: logger_(std::move(logger)) {
	initialize(file_path, open_read_only);
}

auto SQLiteConnection::initialize(std::string_view file_path, bool open_read_only) -> void {
	// initialize a new sqlite3 object (managed by sqlite)
	sqlite3* db_instance = nullptr;
	auto result = sqlite3_open_v2(file_path.data(),
		&db_instance,
		open_read_only ? SQLITE_OPEN_READONLY : SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,  // NOLINT (sqlite has C-API)
		nullptr);
	connection_.reset(db_instance);

	if (result != SQLITE_OK) {
		std::string const error_msg = fmt::format("Failed to open database: '{}'", sqlite3_errstr(result));
		logger_->error(error_msg);
		throw std::runtime_error(error_msg);
	}

	logger_->info("opened ({}) database '{}'", file_path, open_read_only ? "read-only" : "read-write");
}

auto SQLiteConnection::execute(std::string_view statement) const -> tl::expected<SQLResult, Failure> {
	logger_->debug("executing '{}' ...", statement);

	SQLResult result;
	int const result_code = sqlite3_exec(connection_.get(), statement.data(), read_rows_callback, &result, nullptr);
	if (SQLITE_OK == result_code) {
		logger_->debug("... read {} rows of {} columns", result.rows.size(), result.header.size());
		return result;
	}
	return tl::unexpected<Failure>({.code = result_code,
		.reason = fmt::format("{}: {}", sqlite3_errstr(result_code), sqlite3_errmsg(connection_.get()))});
}

}  // namespace tools
