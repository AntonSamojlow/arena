#include "storage_test.h"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <utility>

#include "../helpers.h"
#include "sag/TicTacToe.h"
#include "sag/rec/Match.h"
#include "sag/rec/Player.h"
#include "sag/storage/MemoryMatchStorage.h"
#include "sag/storage/SQLiteMatchStorage.h"

using namespace sag::storage;

namespace {
TEMPLATE_PRODUCT_TEST_CASE("MemoryMatchStorage test",
	"[sag, storage]",
	std::pair,
	((short, int), (int, int), (int, double), (int, std::string))) {
	auto match = create_match<typename TestType::first_type, typename TestType::second_type>();
	MemoryMatchStorage<typename TestType::first_type, typename TestType::second_type> storage;
	CHECK(storage.size() == 0);
	auto result = storage.add(match, "{\"extra_data\": true}");
	CHECK(result.has_value());
	CHECK(storage.size() == 1);
}

TEMPLATE_PRODUCT_TEST_CASE("SQLiteMatchStorage test",
	"[sag, storage]",
	std::pair,
	((IntegerConverter<short>, IntegerConverter<int>),
		(IntegerConverter<int>, IntegerConverter<int>),
		(IntegerConverter<int>, FloatingPointConverter<double>),
		(IntegerConverter<int>, TextConverter))) {
	auto match = create_match<typename TestType::first_type::original, typename TestType::second_type::original>();
	test::TempFilePath const db_file = test::unique_file_path(false);
	auto connection = std::make_unique<tools::SQLiteConnection>(db_file.get(), false);
	SQLiteMatchStorage<typename TestType::first_type, typename TestType::second_type> const storage(
		std::move(connection));

	CHECK(storage.count_matches().value() == 0);
	CHECK(storage.count_records().value() == 0);

	// verify adding a match succeeds
	auto result = storage.add(match, "{\"extra_data\": true}");
	CHECK(result.has_value());
	CHECK(storage.count_matches().value() == 1);
	CHECK(storage.count_records().value() == match.plays.size());

	// verify adding the exact same match succeeds
	result = storage.add(match, "{\"extra_data\": true}");
	CHECK(result.has_value());
	CHECK(storage.count_matches().value() == 2);
	CHECK(storage.count_records().value() == 2 * match.plays.size());

	// verify counting nonexisting table rows fails cleanly
	auto test_connection = std::make_unique<tools::SQLiteConnection>(db_file.get(), false);
	test_connection->execute("DROP TABLE records");
	CHECK(storage.count_matches().has_value());
	CHECK(!storage.count_records().has_value());
	test_connection->execute("DROP TABLE matches");
	CHECK(!storage.count_matches().has_value());
}

}  // namespace
