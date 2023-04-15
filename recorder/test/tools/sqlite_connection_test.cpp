#include <spdlog/common.h>
#include <sqlite3.h>

#include <catch2/catch_test_macros.hpp>
#include <filesystem>

#include "../helpers.h"
#include "tools/SQLiteConnection.h"

TEST_CASE("SQLiteConnectionTest", "[tools]") {
	std::string const insert_command = "insert into tbl1 values('hello!',10);insert into tbl1 values('goodbye', 20);";
	std::string const create_command = "create table tbl1(one text, two int);" + insert_command;
	std::string const read_command = "select * from tbl1;";

	auto logger = spdlog::default_logger();
	logger->set_level(spdlog::level::debug);

	{
		test::TempFilePath const file_path = test::unique_file_path(false);
		// open in read-write mode also creates the database
		REQUIRE(!std::filesystem::exists(file_path.get()));
		tools::SQLiteConnection const handler{file_path.get(), false, logger};

		// create a table
		auto create_result = handler.execute(create_command);
		REQUIRE(create_result.has_value());
		CHECK(create_result->header.empty());
		CHECK(create_result->rows.empty());

		// creating same table again must fail
		auto second_create_result = handler.execute(create_command);
		REQUIRE(!second_create_result.has_value());
		CHECK(second_create_result.error().code == SQLITE_ERROR);

		// read from the table
		auto read_result = handler.execute(read_command);
		REQUIRE(create_result.has_value());
		CHECK(read_result->header.size() == 2);
		auto const& rows = read_result->rows;
		REQUIRE(rows.size() == 2);
		CHECK(rows[0][0] == "hello!");
		CHECK(rows[0][1] == "10");
		CHECK(rows[1][0] == "goodbye");
		CHECK(rows[1][1] == "20");

		// create a second connection to same file and read from the table
		tools::SQLiteConnection const second_handler{file_path.get(), false, logger};
		auto second_read_result = second_handler.execute(read_command);
		REQUIRE(second_read_result.has_value());
		CHECK(second_read_result->header.size() == 2);
		auto const& second_rows = second_read_result->rows;
		REQUIRE(second_rows.size() == 2);
		CHECK(second_rows[0][0] == "hello!");
		CHECK(second_rows[0][1] == "10");
		CHECK(second_rows[1][0] == "goodbye");
		CHECK(second_rows[1][1] == "20");
	}

	{
		// file exists: opening read-only succeeds
		test::TempFilePath const file_path = test::unique_file_path(true);
		CHECK(std::filesystem::exists(file_path.get()));
		tools::SQLiteConnection const handler{file_path.get(), true};

		// creating table fails
		auto create_result = handler.execute(create_command);
		REQUIRE(!create_result.has_value());
		CHECK(create_result.error().code == SQLITE_READONLY);
	}

	{
		// file missing: opening read-only fails
		test::TempFilePath const file_path = test::unique_file_path(false);
		CHECK(!std::filesystem::exists(file_path.get()));
		CHECK_THROWS(tools::SQLiteConnection{file_path.get(), true});
	}
}
