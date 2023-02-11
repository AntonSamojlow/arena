#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "sag/TicTacToe.h"
#include "sag/match/Match.h"
#include "sag/match/MemoryStorage.h"
#include "sag/match/Player.h"

using namespace sag::tic_tac_toe;
namespace {

TEST_CASE("MemoryStorage test", "[storage]") {
	Graph::container container;
	Graph::rules const rules;

	sag::match::RandomPlayer<Graph> player_one;
	sag::match::RandomPlayer<Graph> player_two;
	sag::match::MatchRecorder recorder{};
	auto root = container.roots()[0];

	auto result = recorder.record_duel<Graph>(root, container, rules, player_one, player_two);

	sag::match::MemoryStorage<Graph::state, Graph::action> storage;
	CHECK(storage.size() == 0);
	storage.add(result, "{}");
	CHECK(storage.size() == 1);
}
}  // namespace
