#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <sqlite3.h>

#include <catch2/catch_test_macros.hpp>
#include <filesystem>

#include "sag/tools/SQLiteHandler.h"

TEST_CASE("SQLiteHandlerTest", "[sqlite_handler]") {
	std::string test_db_file = "test.db";
	std::string insert_command = "insert into tbl1 values('hello!',10);insert into tbl1 values('goodbye', 20);";
	std::string create_command = "create table tbl1(one text, two int);" + insert_command;
	std::string read_command = "select * from tbl1;";

	spdlog::default_logger()->set_level(spdlog::level::debug);

	std::filesystem::remove(test_db_file);
	{
		// open in read-write mode also creates the database
		CHECK(!std::filesystem::exists(test_db_file));
		tools::SQLiteHandler handler{test_db_file, false};

		// create a table
		auto create_result = handler.execute(create_command);
		CHECK(create_result.result_code == SQLITE_OK);
		CHECK(create_result.header.empty());
		CHECK(create_result.rows.empty());

		// creating same table again must fail
		auto second_create_result = handler.execute(create_command);
		CHECK(second_create_result.result_code != SQLITE_OK);

		// read from the table
		auto read_result = handler.execute(read_command);
		CHECK(read_result.result_code == SQLITE_OK);
		CHECK(read_result.header.size() == 2);
		CHECK(read_result.rows.size() == 2);
	}

	{
		// file exists: opening read-only succeeds
		CHECK(std::filesystem::exists(test_db_file));
		tools::SQLiteHandler handler{test_db_file, true};

		// read from the table
		auto read_result = handler.execute(read_command);
		CHECK(read_result.result_code == SQLITE_OK);
		CHECK(read_result.header.size() == 2);
		CHECK(read_result.rows.size() == 2);
		auto insert_result = handler.execute(insert_command);
		CHECK(insert_result.result_code != SQLITE_OK);
	}

	CHECK(std::filesystem::exists(test_db_file));
	std::filesystem::remove(test_db_file);

	// file missing: opening read-only fails
	CHECK(!std::filesystem::exists(test_db_file));
	CHECK_THROWS(tools::SQLiteHandler{test_db_file, true});
}
