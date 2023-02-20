#include <spdlog/spdlog.h>
#include <sqlite3.h>

#include <cstddef>
#include <string>


// static auto callback(void* /*NotUsed*/, int argc, char **argv, char **azColName) -> int {
// 	auto logger = spdlog::default_logger();

// 	int i;
// 	for (i = 0; i < argc; i++) {
// 		logger->info("{} = {}", azColName[i], argv[i] ? argv[i] : "NULL");
// 	}
// 	logger->info("");
// 	return 0;
// }

// auto test_sqlite() -> void {
// 	auto logger = spdlog::default_logger();

// 	sqlite3 *db;
// 	char *zErrMsg = 0;
// 	int rc;

// 	std::string file_name = "new_test_3.db";
// 	rc = sqlite3_open_v2(file_name.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);

// 	std::string create_command = "create table tbl1(one text, two int);insert into tbl1 values('hello!',10);insert into tbl1 values('goodbye', 20);";
// 	std::string read_command = "select * from tbl1;";

// 	if (rc == SQLITE_OK) {
// 		if(SQLITE_OK != sqlite3_exec(db, create_command.c_str(), callback, 0, &zErrMsg))
// 			logger->error(zErrMsg);
// 		if(SQLITE_OK != sqlite3_exec(db, read_command.c_str(), callback, 0, &zErrMsg))
// 			logger->error(zErrMsg);
// 	}
// 	sqlite3_free(zErrMsg);
// 	sqlite3_close(db);
// }


auto main() -> int {
	auto logger = spdlog::default_logger();
	logger->info("recorder start");
	// test_sqlite();
	logger->info("recorder end");
	return 0;
}


