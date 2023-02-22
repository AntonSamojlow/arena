#include "SQLiteHandler.h"

#include <fmt/core.h>
#include <spdlog/logger.h>
#include <sqlite3.h>

#include <exception>
#include <memory>
#include <string>
#include <utility>

#include "tools/SQLiteHandler.h"


// NOLINTBEGIN
namespace {

/// callback that both reads rows and logs
auto read_rows_callback(void* input_ptr, int argc, char** argv, char** azColName) -> int {
	tools::ExecuteResult& result = *static_cast<tools::ExecuteResult*>(input_ptr);

	if (result.header.empty()) {
		result.header.reserve(static_cast<size_t>(argc));
		for (int i = 0; i < argc; i++) {
			result.header.emplace_back(azColName[i]);
		}
	}

	std::vector<std::string> row;
	row.reserve(static_cast<size_t>(argc));
	for (int i = 0; i < argc; i++) {
		row.emplace_back(argv[i]);
	}
	result.rows.push_back(row);
	return 0;
}

}  // namespace

namespace tools {

SQLiteHandler::SQLiteHandler(std::string_view file_path, bool open_read_only) {
	initialize(file_path, open_read_only);
}

SQLiteHandler::SQLiteHandler(std::string_view file_path, bool open_read_only, std::shared_ptr<spdlog::logger> logger)
		: logger_(std::move(logger)) {
	initialize(file_path, open_read_only);
}

auto SQLiteHandler::initialize(std::string_view file_path, bool open_read_only) -> void {
	// initialize a new sqlite3 object (managed by sqlite)
	sqlite3* db_instance = nullptr;
	auto result = sqlite3_open_v2(file_path.data(),
		&db_instance,
		open_read_only ? SQLITE_OPEN_READONLY : SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
		nullptr);
	db_handle_.reset(db_instance);

	if (result != SQLITE_OK) {
		std::string error_msg = fmt::format("Failed to open database: '{}'", sqlite3_errstr(result));
		logger_->error(error_msg);
		throw std::exception(error_msg.c_str());
	}

	logger_->info("opened ({}) database '{}'", file_path, open_read_only ? "read-only" : "read-write");
}

auto SQLiteHandler::execute(std::string_view statement) -> ExecuteResult {
	logger_->debug("executing '{}' ...", statement);

	ExecuteResult result;
	result.result_code = sqlite3_exec(db_handle_.get(), statement.data(), read_rows_callback, &result, nullptr);
	if (SQLITE_OK != result.result_code) {
		result.error = fmt::format("{}: {}", sqlite3_errstr(result.result_code), sqlite3_errmsg(db_handle_.get()));
		logger_->error(result.error);
	}

	logger_->debug("... read {} rows of {} columns", result.rows.size(), result.header.size());
	return result;
}

// NOLINTEND

}  // namespace tools
