#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "../helpers.h"
#include "sag/TicTacToe.h"
#include "sag/match/MatchRecorder.h"
#include "sag/match/RandomPlayer.h"
#include "sag/storage/SQLiteMatchStorage.h"

using namespace sag::tic_tac_toe;
using namespace sag::match;
using namespace std::chrono;

namespace {

TEST_CASE("Match recorder test", "[sag, match]") {
	struct TestRecorderTypes {
		using graph = Graph;
		using player = RandomPlayer<Graph>;
		using storage = sag::storage::SQLiteMatchStorage<sag::storage::IntegerConverter<typename graph::state>,
			sag::storage::IntegerConverter<typename graph::action>>;
	};

	static_assert(std::is_nothrow_move_constructible_v<RecorderThreadHandle<TestRecorderTypes>>);
	static_assert(std::is_nothrow_move_assignable_v<RecorderThreadHandle<TestRecorderTypes>>);

	test::TempFilePath const db_file = test::unique_file_path(false);
	TestRecorderTypes::storage const test_storage(std::make_unique<tools::SQLiteConnection>(db_file.get(), false));

	// start several recorder threads
	std::vector<RecorderThreadHandle<TestRecorderTypes>> recorder_threads;
	recorder_threads.reserve(3);
	for (int i = 0; i < 3; ++i) {
		// assemble recorder with owning storage
		auto connection = std::make_unique<tools::SQLiteConnection>(db_file.get(), false);
		TestRecorderTypes::storage storage(std::move(connection));
		MatchRecorder<TestRecorderTypes> recorder{{{}, {}}, {}, {}, std::move(storage)};
		recorder_threads.emplace_back(std::move(recorder));
	}

	auto signal_all = [&recorder_threads](Signal signal) -> void {
		for (auto& rec : recorder_threads)
			rec.queue().emplace(signal);
	};

	std::this_thread::sleep_for(100ms);
	CHECK(test_storage.count_records().value() == 0);
	CHECK(test_storage.count_matches().value() == 0);
	signal_all(Signal::Status);
	recorder_threads[0].queue().emplace(Signal::Record);
	std::this_thread::sleep_for(100ms);

	signal_all(Signal::Status);
	std::this_thread::sleep_for(100ms);

	recorder_threads[0].queue().emplace(Signal::Halt);
	signal_all(Signal::Status);
	std::this_thread::sleep_for(100ms);

	auto match_count_one_thread = test_storage.count_matches().value();
	CHECK(match_count_one_thread > 0);
	std::this_thread::sleep_for(100ms);
	bool match_count_is_constant = match_count_one_thread == test_storage.count_matches().value();
	CHECK(match_count_is_constant);

	signal_all(Signal::Record);
	std::this_thread::sleep_for(100ms);

	signal_all(Signal::Quit);
	std::this_thread::sleep_for(100ms);

	auto match_count_several_threads = test_storage.count_matches().value();
	CHECK(match_count_several_threads > match_count_one_thread);
	std::this_thread::sleep_for(100ms);
	match_count_is_constant = match_count_several_threads == test_storage.count_matches().value();
	CHECK(match_count_is_constant);
}
}  // namespace
