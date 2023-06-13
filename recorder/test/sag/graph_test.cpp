#include "graph_test.h"

#include <catch2/catch_template_test_macros.hpp>

#include "sag/ExampleGraph.h"
#include "sag/TicTacToe.h"
#include "sag/santorini/Graph.h"

constexpr sag::santorini::Dimensions santorini_2x2_1 = {.rows = 2, .cols = 2, .player_unit_count = 1};
constexpr sag::santorini::Dimensions santorini_3x5_2 = {.rows = 3, .cols = 5, .player_unit_count = 2};

struct ExampleGraphCollection {
	using state = int;
	using action = int;
	using container = sag::example::Container;
	using rules = sag::example::Rules;
	using printer = sag::example::Container;

	ExampleGraphCollection() = default;

	[[nodiscard]] auto get_rules() const -> rules { return rules_; }
	[[nodiscard]] auto get_container() const -> container { return container_; }
	[[nodiscard]] auto get_printer() const -> printer { return printer_; }

 private:
	std::vector<sag::example::ActionEdges> TERMINAL_ = {};

	rules rules_{sag::example::GraphStructure{
		{1,
			{
				{{1.0F / 3, 2}, {2.0F / 3, 3}},
				{{0.25F, 5}, {0.75F, 6}},
			}},
		{2, TERMINAL_},
		{3, {{{1.0, 4}}}},
		{4, TERMINAL_},
		{5, TERMINAL_},
		{6, {{{0.5, 7}, {0.5, 8}}}},
		{7, TERMINAL_},
		{8, {{{1.0, 9}}}},
		{9, TERMINAL_},
	}};

	container container_{rules_};
	printer printer_;
};

TEMPLATE_TEST_CASE("Graph test",
	"[sag]",
	ExampleGraphCollection,
	Defaulted<sag::tic_tac_toe::Graph>,
	Defaulted<sag::santorini::Graph<santorini_2x2_1>>,
	Defaulted<sag::santorini::Graph<santorini_3x5_2>>) {
	static_assert(TestGraphCollection<TestType>);

	TestType testType;
	typename TestType::container graph = testType.get_container();
	typename TestType::rules const rules = testType.get_rules();
	typename TestType::printer const printer = testType.get_printer();

	// test and assert
	test_base_operations<typename TestType::state, typename TestType::action>(graph, rules);
	std::vector<typename TestType::state> visited_states{};
	test_full_descend<typename TestType::state, typename TestType::action>(graph, rules, true, visited_states);

	// write test log
	auto logger = spdlog::default_logger();
	logger->info("first root and its actions:");
	auto root = graph.roots().front();
	for (auto action : rules.list_actions(root)) {
		logger->info(printer.to_string(root, action));
	}
	logger->info("logging all states of a full descend:");
	for (auto state : visited_states)
		logger->info("\n{}, score: {}", printer.to_string(state), rules.score(state).value());
}
