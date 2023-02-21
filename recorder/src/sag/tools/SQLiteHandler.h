#pragma once

#include <spdlog/spdlog.h>
#include <sqlite3.h>

#include <concepts>
#include <memory>
#include <string_view>
#include <vector>

#include "UniqueResource.h"

namespace tools {

class SQLiteHandler {
	struct db_deleter {
		auto operator()(sqlite3* db_handle) -> void { sqlite3_close(db_handle); };
	};
	struct msg_buffer_deleter {
		auto operator()(char* zErrMsg) -> void { sqlite3_free(zErrMsg); };
	};

 public:
	explicit SQLiteHandler(std::string_view file_path);

	auto execute(std::string_view statement) -> bool;
	auto execute_2(std::string_view statement) -> std::vector<std::vector<std::string>>;

 private:
	unique_resource<sqlite3*, db_deleter> db_handle_ = {nullptr, db_deleter{}};
	unique_resource<char*, msg_buffer_deleter> error_buffer_ = {nullptr, msg_buffer_deleter{}};
	;
	std::shared_ptr<spdlog::logger> logger_ = spdlog::default_logger();
};

}  // namespace tools
