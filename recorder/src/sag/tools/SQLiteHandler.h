#pragma once

#include <spdlog/spdlog.h>
#include <sqlite3.h>

#include <memory>
#include <string_view>
#include <vector>

namespace tools {

struct ExecuteResult {
	std::vector<std::string> header;
	std::vector<std::vector<std::string>> rows;
	int result_code;
	std::string error;
};

class SQLiteHandler {
	struct db_deleter {
		auto operator()(sqlite3* db_handle) -> void { sqlite3_close_v2(db_handle); }
	};

 public:
	explicit SQLiteHandler(std::string_view file_path, bool open_read_only);
	SQLiteHandler(std::string_view file_path, bool open_read_only, std::shared_ptr<spdlog::logger> logger);

	auto execute(std::string_view statement) -> ExecuteResult;

 private:
	auto initialize(std::string_view file_path, bool open_read_only) -> void;
	std::unique_ptr<sqlite3, db_deleter> db_handle_;
	std::shared_ptr<spdlog::logger> logger_ = spdlog::default_logger();
};

}  // namespace tools
