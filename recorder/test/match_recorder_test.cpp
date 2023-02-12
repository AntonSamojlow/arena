#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <chrono>

#include "sag/TicTacToe.h"
#include "sag/match/Match.h"
#include "sag/match/MemoryStorage.h"
#include "sag/match/Player.h"

using namespace sag::tic_tac_toe;
using namespace std::chrono;
namespace {

TEST_CASE("Match recorder test", "[match]") {
	Graph::container container;
	Graph::rules const rules;

	sag::match::RandomPlayer<Graph> player_one{};
	sag::match::RandomPlayer<Graph> player_two{};

	sag::match::MatchRecorder recorder{};

	auto root = container.roots()[0];
	auto const time_pre_record = steady_clock::now();
	auto result = recorder.record_duel<Graph>(root, container, rules, player_one, player_two);
	auto const time_post_record = steady_clock::now();

	CHECK(time_pre_record < result.start);
	CHECK(result.start < result.end);
	CHECK(result.end < time_post_record);

	CHECK(result.player_ids[0] == player_one.id());
	CHECK(result.player_ids[1] == player_two.id());

	for (auto play : result.plays)
		spdlog::default_logger()->info("state {}", container.to_string(play.state()));
	spdlog::default_logger()->info("endstate: {}", container.to_string(result.end_state));
	CHECK(result.plays.size() > 4);
	CHECK(result.plays.size() < 10);

	float const end_value = rules.score(result.end_state).value();
	bool const is_a_draw = std::abs(end_value) < std::numeric_limits<float>::epsilon();
	bool const is_a_loss = std::abs(end_value + 1.0F) < std::numeric_limits<float>::epsilon();
	CHECK((is_a_draw || is_a_loss));
}
}  // namespace
