#pragma once

#include <spdlog/spdlog.h>
#include <sqlite3.h>

#include <memory>
#include <string_view>
#include <tl/expected.hpp>
#include <vector>

#include "Failure.h"

namespace tools {

struct SQLResult {
	std::vector<std::string> header;
	std::vector<std::vector<std::string>> rows;
};

/// Custom wrapper around three standard sqlite3 functions:
/// ctor <-> sqlite3_open_v2 | dtor <-> sqlite3_close_v2 | execute <-> sqlite3_exec
class SQLiteConnection {
	struct db_deleter {
		auto operator()(sqlite3* db_handle) const -> void { sqlite3_close_v2(db_handle); }
	};

 public:
	explicit SQLiteConnection(std::string_view file_path, bool open_read_only);
	SQLiteConnection(std::string_view file_path, bool open_read_only, std::shared_ptr<spdlog::logger> logger);

	// NOLINTNEXTLINE(modernize-use-nodiscard)
	auto execute(std::string_view statement) const -> tl::expected<SQLResult, Failure>;

 private:
	auto initialize(std::string_view file_path, bool open_read_only) -> void;
	std::unique_ptr<sqlite3, db_deleter> connection_;
	std::shared_ptr<spdlog::logger> logger_ = spdlog::default_logger();
};

}  // namespace tools
