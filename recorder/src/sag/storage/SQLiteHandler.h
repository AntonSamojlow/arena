#pragma once

#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <sqlite3.h>

#include <memory>
#include <string_view>

namespace sag::storage {

class SQLiteHandler {
 public:
	explicit SQLiteHandler(std::string_view file_path);

 private:
	std::unique_ptr<sqlite3> db_handle_;
	std::shared_ptr<spdlog::logger> logger_ = spdlog::default_logger();
};

}  // namespace sag::storage
