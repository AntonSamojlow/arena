#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <chrono>

#include "sag/TicTacToe.h"
#include "sag/match/Match.h"
#include "sag/match/Player.h"
#include "sag/storage/MemoryMatchStorage.h"

using namespace sag::tic_tac_toe;
using namespace std::chrono;
namespace {

TEST_CASE("Match recorder test", "[sag, match]") {
	Graph::container container;
	Graph::rules const rules;

	sag::match::RandomPlayer<Graph> player_one{};
	sag::match::RandomPlayer<Graph> player_two{};

	sag::match::MatchRecorder recorder{};

	auto root = container.roots()[0];
	auto const time_pre_record = steady_clock::now();
	auto match = recorder.record_duel<Graph>(root, container, rules, player_one, player_two);
	auto const time_post_record = steady_clock::now();

	CHECK(time_pre_record < match.start);
	CHECK(match.start < match.end);
	CHECK(match.end < time_post_record);

	CHECK(match.player_ids[0] == player_one.id());
	CHECK(match.player_ids[1] == player_two.id());

	for (auto play : match.plays)
		spdlog::default_logger()->info("state {}", container.to_string(play.state()));
	spdlog::default_logger()->info("endstate: {}", container.to_string(match.end_state));
	CHECK(match.plays.size() > 4);
	CHECK(match.plays.size() < 10);

	float const end_value = rules.score(match.end_state).value();
	bool const is_a_draw = std::abs(end_value) < std::numeric_limits<float>::epsilon();
	bool const is_a_loss = std::abs(end_value + 1.0F) < std::numeric_limits<float>::epsilon();
	CHECK((is_a_draw || is_a_loss));

	sag::storage::MemoryMatchStorage<typename Graph::state, typename Graph::action> storage{};
	CHECK(storage.size() == 0);
	storage.add(match, "some_extra_data");
	CHECK(storage.size() == 1);
}
}  // namespace
