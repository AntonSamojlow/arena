#include "sag/mcts/MCTS.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "sag/ExampleGraph.h"
#include "sag/mcts/MCTSConcepts.h"
#include "sag/mcts/StatsContainer.h"

using namespace sag::example;

namespace {

/// Advanded example graph with (true) action values at root '1' are: 1/3 and 1/2
auto create_wide_graph_rules() -> ExampleRulesEngine {
	std::vector<sag::example::ActionEdges> const TERMINAL = {};
	sag::example::GraphStructure const graph_structure = {
		{1,
			{
				{{1.0F / 3, 2}, {2.0F / 3, 4}},
				{{0.25F, 8}, {0.75F, 9}},
			}},
		{2, {{{1.0F, 3}}}},
		{3, TERMINAL},
		{4, {{{0.5F, 5}, {0.5F, 6}}}},
		{5, TERMINAL},
		{6, {{{1.0F, 7}}}},
		{7, TERMINAL},
		{8, TERMINAL},
		{9,
			{
				{{0.5F, 10}, {0.5F, 11}},
				{{1.0F, 13}},
				{{0.25F, 14}, {0.25F, 15}, {0.5F, 18}},
			}},
		{10, TERMINAL},
		{11, {{{1.0F, 12}}}},
		{12, TERMINAL},
		{13, TERMINAL},
		{14, TERMINAL},
		{15,
			{
				{{1.0F, 16}},
				{{1.0F, 17}},
			}},
		{16, TERMINAL},
		{17, TERMINAL},
		{18, {{{1.0F, 19}}, {{1.0F, 20}}}},
		{19, TERMINAL},
		{20, {{{1.0F, 21}}}},
		{21, TERMINAL},
	};

	return ExampleRulesEngine(graph_structure);
}

/// Simple example graph with (true) action values at root '1' are: 1/3 and -1/4
auto create_small_graph_rules() -> ExampleRulesEngine {
	std::vector<ActionEdges> const TERMINAL = {};
	GraphStructure const graph_structure = {
		{1,
			{
				{{1.0F / 3, 2}, {2.0F / 3, 3}},
				{{0.25F, 5}, {0.75F, 6}},
			}},
		{2, TERMINAL},
		{3, {{{1.0, 4}}}},
		{4, TERMINAL},
		{5, TERMINAL},
		{6, {{{0.5, 7}, {0.5, 8}}}},
		{7, TERMINAL},
		{8, {{{1.0, 9}}}},
		{9, TERMINAL},
	};
	return ExampleRulesEngine(graph_structure);
}

/// Check that base MCTS finds the correct action values at root
auto test_small_graph(bool uniform_action_sampling) -> void {
	ExampleRulesEngine const rules = create_small_graph_rules();
	ExampleGraph graph(rules);
	sag::mcts::Statistics<State> stats;

	// choose a low exploration, since there is not much to explore
	sag::mcts::BaseMCTS<State, Action> mcts_algo(uniform_action_sampling, sag::NonNegative(0.1F));
	State const root = graph.roots()[0];

	// Test BaseMCTS finds the correct action values at root
	CHECK(stats.size() == 0);
	for (size_t i = 0; i < 10'000; i++) {
		mcts_algo.descend(root, stats, graph, rules);
	}
	REQUIRE(stats.size() > 0);
	auto result = sag::mcts::action_estimates_at<State, Action>(root, graph, stats);
	REQUIRE(result.size() == 2);
	CHECK_THAT(result[0], Catch::Matchers::WithinAbs(1.0 / 3, 0.001));
	CHECK_THAT(result[1], Catch::Matchers::WithinAbs(-0.25, 0.02));
}
}  // namespace

TEST_CASE("BaseMCTS test (uniform action sampling)", "[mcts]") {
	test_small_graph(true);
}

TEST_CASE("BaseMCTS test (random action sampling)", "[mcts]") {
	test_small_graph(false);
}

TEST_CASE("BaseMCTS test (wide graph)", "[mcts]") {
	// Check that base MCTS finds the correct action values at root
	ExampleRulesEngine const rules = create_wide_graph_rules();
	ExampleGraph graph(rules);
	sag::mcts::Statistics<State> stats;
	sag::mcts::BaseMCTS<State, Action> mcts_default{};
	sag::mcts::BaseMCTS<State, Action> mcts_low_exploration{false, sag::NonNegative(0.1F)};
	State const root = graph.roots()[0];

	CHECK(stats.size() == 0);
	for (size_t i = 0; i < 1'000; i++) {
		mcts_default.descend(root, stats, graph, rules);
	}
	for (size_t i = 0; i < 10'000; i++) {
		mcts_low_exploration.descend(root, stats, graph, rules);
	}
	REQUIRE(stats.size() > 0);
	auto result = sag::mcts::action_estimates_at<State, Action>(root, graph, stats);
	REQUIRE(result.size() == 2);
	CHECK_THAT(result[0], Catch::Matchers::WithinAbs(1.0 / 3, 0.1));
	CHECK_THAT(result[1], !Catch::Matchers::WithinAbs(0.5, 0.05));

	// run one more round to improve both estimates
	for (size_t i = 0; i < 10'000; i++) {
		mcts_low_exploration.descend(root, stats, graph, rules);
	}
	result = sag::mcts::action_estimates_at<State, Action>(root, graph, stats);
	REQUIRE(result.size() == 2);
	CHECK_THAT(result[0], Catch::Matchers::WithinAbs(1.0 / 3, 0.05));
	CHECK_THAT(result[1], Catch::Matchers::WithinAbs(0.5, 0.05));
}

TEST_CASE("MCTS update test", "[mcts]") {
	sag::mcts::Statistics<State> stats;
	stats.initialize(1, sag::Score(0.1F));
	stats.initialize(2, sag::Score(0.2F));
	stats.initialize(3, sag::Score(0.3F));
	stats.initialize(4, sag::Score(0.4F));

	auto check_initial_state = [&]() -> void {
		CHECK(stats.size() == 4);
		CHECK_THAT(stats.at(1).Q, Catch::Matchers::WithinRel(0.1F));
		CHECK_THAT(stats.at(2).Q, Catch::Matchers::WithinRel(0.2F));
		CHECK_THAT(stats.at(3).Q, Catch::Matchers::WithinRel(0.3F));
		CHECK_THAT(stats.at(4).Q, Catch::Matchers::WithinRel(0.4F));
		CHECK(stats.at(1).N == 0);
		CHECK(stats.at(2).N == 0);
		CHECK(stats.at(3).N == 0);
		CHECK(stats.at(4).N == 0);
	};

	check_initial_state();
	sag::mcts::Path<State> path;
	path.terminal = false;

	// updating with empty pathshould not change the stats
	sag::mcts::update(path, stats);
	check_initial_state();

	// do a real update
	path.values = {2, 4};
	sag::mcts::update(path, stats);

	// expect node 2 to have an updated average of "(old_val*(N-1) + new_val) /N"
	// with N=1, old_val=0.2F, new_val=-0.4F, hence in total: -0.4F in total
	CHECK_THAT(stats.at(2).Q, Catch::Matchers::WithinRel(-0.4F));
	CHECK(stats.at(2).N == 1);

	// expect node 4 to have an added visit but no value change
	CHECK_THAT(stats.at(4).Q, Catch::Matchers::WithinRel(0.4F));
	CHECK(stats.at(4).N == 1);

	// expect all other nodes to be unchanged
	CHECK_THAT(stats.at(1).Q, Catch::Matchers::WithinRel(0.1F));
	CHECK_THAT(stats.at(3).Q, Catch::Matchers::WithinRel(0.3F));
	CHECK(stats.at(1).N == 0);
	CHECK(stats.at(3).N == 0);

	// update once more, now with a path of 2-to-3
	sag::mcts::Path<State> second_path;
	second_path.values = {2, 3};
	sag::mcts::update(second_path, stats);

	// expect node 2 to have an updated average of "(old_val*(N-1) + new_val) /N"
	// with N=2, old_val=-0.4F, new_val=-0.3F, hence in total: -0.35F
	CHECK_THAT(stats.at(2).Q, Catch::Matchers::WithinRel(-0.35F));
	CHECK(stats.at(2).N == 2);

	// expect node 3 to have an added visit but no value change
	CHECK_THAT(stats.at(3).Q, Catch::Matchers::WithinRel(0.3F));
	CHECK(stats.at(3).N == 1);

	// expect all other nodes to be unchanged
	CHECK_THAT(stats.at(1).Q, Catch::Matchers::WithinRel(0.1F));
	CHECK_THAT(stats.at(4).Q, Catch::Matchers::WithinRel(0.4F));
	CHECK(stats.at(1).N == 0);
	CHECK(stats.at(4).N == 1);
}
