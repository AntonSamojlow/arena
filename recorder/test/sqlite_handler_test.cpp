#include <catch2/catch_test_macros.hpp>
#include <filesystem>

#include "sag/tools/SQLiteHandler.h"

TEST_CASE("SQLiteHandlerTest", "[sqlite_handler]") {
	std::string test_db_file = "test.db";

	std::filesystem::remove(test_db_file);
	CHECK(!std::filesystem::exists(test_db_file));

	{
		tools::SQLiteHandler handler{test_db_file};
		std::string create_command =
			"create table tbl1(one text, two int);insert into tbl1 values('hello!',10);insert into tbl1 values('goodbye', "
			"20);";
		std::string read_command = "select * from tbl1;";
		handler.execute(create_command);
		handler.execute(read_command);
		auto result = handler.execute_2(read_command);
		CHECK(result.size() == 3);
		CHECK(result[0].size() == 2);
	}
	CHECK(std::filesystem::exists(test_db_file));

	std::filesystem::remove(test_db_file);
	CHECK(!std::filesystem::exists(test_db_file));
}
