#include <catch2/catch_test_macros.hpp>

#include "sag/TicTacToe.h"
#include "sag/match/Match.h"
#include "sag/match/Player.h"

using namespace sag::tic_tac_toe;
namespace {

TEST_CASE("Match recorder test", "[match]") {
	Graph::container container;
	Graph::rules const rules;

	sag::match::RandomPlayer<Graph> player_one;
	sag::match::RandomPlayer<Graph> player_two;

	sag::match::MatchRecorder recorder{};

	auto root = container.roots()[0];
	auto result = recorder.record_duel<Graph>(root, container, rules, player_one, player_two);
	for (auto play : result.plays)
		spdlog::default_logger()->info("state {}", container.to_string(play.state()));
	spdlog::default_logger()->info("endstate: {}", container.to_string(result.end_state));
	CHECK(result.plays.size() > 4);
	CHECK(result.plays.size() < 10);
	CHECK(rules.score(result.end_state).value() == -1.0F);
}
}  // namespace