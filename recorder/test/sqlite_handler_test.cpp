#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <sqlite3.h>

#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <memory>

#include "sag/storage/SQLiteStorage.h"
#include "sag/tools/SQLiteConnection.h"

TEST_CASE("SQLiteHandlerTest", "[sqlite_handler]") {
	std::string const test_db_file = "test.db";
	std::string const insert_command = "insert into tbl1 values('hello!',10);insert into tbl1 values('goodbye', 20);";
	std::string const create_command = "create table tbl1(one text, two int);" + insert_command;
	std::string const read_command = "select * from tbl1;";

	spdlog::default_logger()->set_level(spdlog::level::debug);

	std::filesystem::remove(test_db_file);
	{
		// open in read-write mode also creates the database
		REQUIRE(!std::filesystem::exists(test_db_file));
		tools::SQLiteConnection handler{test_db_file, false};

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
		CHECK(read_result->rows.size() == 2);
	}

	{
		// file exists: opening read-only succeeds
		CHECK(std::filesystem::exists(test_db_file));
		tools::SQLiteConnection handler{test_db_file, true};

		// read from the table
		auto read_result = handler.execute(read_command);
		REQUIRE(read_result.has_value());
		CHECK(read_result->header.size() == 2);
		CHECK(read_result->rows.size() == 2);
		auto insert_result = handler.execute(insert_command);
		REQUIRE(!insert_result.has_value());
		CHECK(insert_result.error().code == SQLITE_READONLY);
	}

	REQUIRE(std::filesystem::exists(test_db_file));
	std::filesystem::remove(test_db_file);

	// file missing: opening read-only fails
	CHECK(!std::filesystem::exists(test_db_file));
	CHECK_THROWS(tools::SQLiteConnection{test_db_file, true});

	{
		auto connection = std::make_unique<tools::SQLiteConnection>(test_db_file, false);
	}
	// sag::storage::SQLMatchStorage<int, int, tools::SQLiteConnection> sql_storage{std::move(connection)};
	std::filesystem::remove(test_db_file);
}
