#include "sag/match/Player.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "sag/ExampleGraph.h"

using namespace sag::example;
namespace {

TEST_CASE("Random player test", "[player]") {
	sag::match::RandomPlayer<Graph> player_one;
	sag::match::RandomPlayer<Graph> player_two;

	SECTION("comparison check") {
		CHECK(player_one == player_two);
		CHECK((player_one != player_two) == false);
	}

	SECTION("random play check") {
		std::vector<ActionEdges> const TERMINAL = {};
		GraphStructure const graph_structure = {
			{1,
				{
					{{1.0, 2}},
					{{1.0, 3}},
				}},
			{2, TERMINAL},
			{3, {{{1.0, 4}}}},
			{4, {TERMINAL}},
		};
		Graph::rules const rules{graph_structure};
		Graph::container graph{rules};

		std::vector<Graph::action> plays;
		plays.reserve(100);
		for (size_t i = 0; i < 100; ++i) {
			plays.push_back(player_one.choose_play(graph.roots()[0], graph, rules));
		}

		// verify that random play chooses between both available actions
		CHECK(std::ranges::find(plays, 0) != plays.end());
		CHECK(std::ranges::find(plays, 1) != plays.end());
	}
}
}  // namespace
