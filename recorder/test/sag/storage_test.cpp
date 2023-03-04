#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <filesystem>

#include "sag/TicTacToe.h"
#include "sag/match/Match.h"
#include "sag/match/Player.h"
#include "sag/storage/MemoryMatchStorage.h"
#include "sag/storage/SQLiteMatchStorage.h"

namespace {

TEST_CASE("Storage test", "[sag, storage]") {
	auto start = std::chrono::steady_clock::now();
	sag::match::Match<int, int> match = {
		.player_ids = {"player-1", "player-2", "player-3"},
		.start = start,
		.end = std::chrono::steady_clock::now(),
		.plays = {{1, 0}, {2, 3}, {3, 1}},
		.end_state = 4,
		.end_score = sag::Score(-1),
	};

	SECTION("Testing MemoryMatchStorage") {
		sag::storage::MemoryMatchStorage<int, int> storage;
		CHECK(storage.size() == 0);
		auto result = storage.add(match, "{}");
		CHECK(result.has_value());
		CHECK(storage.size() == 1);
	}

	SECTION("Testing SQLiteMatchStorage") {
		std::string const test_db_file = "storage_test.db";
		std::filesystem::remove(test_db_file);
		{
			auto connection = std::make_unique<tools::SQLiteConnection>(test_db_file, false);
			sag::storage::SQLiteMatchStorage<sag::storage::IntegerConverter<int>, sag::storage::IntegerConverter<int>>
				storage(std::move(connection));
			auto result = storage.add(match, "{}");
			CHECK(result.has_value());
		}
		std::filesystem::remove(test_db_file);
	}
}

}  // namespace
