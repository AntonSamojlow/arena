#include "SQLiteHandler.h"

#include <spdlog/logger.h>

#include <array>
#include <exception>
#include <string>
#include <vector>

#include "tools/SQLiteHandler.h"

namespace {
auto logging_callback(void* logger_ptr, int argc, char** argv, char** azColName) -> int {
	auto* logger = static_cast<spdlog::logger*>(logger_ptr);
	for (int i = 0; i < argc; i++) {
		logger->info("{} = {}", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	logger->info("");
	return 0;
}
}  // namespace

namespace tools {

SQLiteHandler::SQLiteHandler(std::string_view file_path) {
	sqlite3* db_instance = nullptr;
	auto result = sqlite3_open_v2(file_path.data(), &db_instance, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
	db_handle_ = {db_instance, db_deleter{}};
	if (result != SQLITE_OK)
		throw std::exception("Failed to open database");
	logger_->info("opened database '{}'", file_path);
}

auto SQLiteHandler::execute(std::string_view statement) -> bool {
	char* errmsg = &*error_buffer_;
	if (SQLITE_OK != sqlite3_exec(db_handle_.get(), statement.data(), logging_callback, logger_.get(), &errmsg)) {
		logger_->error(errmsg);
		return false;
	}
	return true;
}

}  // namespace tools
