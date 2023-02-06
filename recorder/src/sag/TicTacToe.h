#pragma once
#include <array>

#include "DefaultGraphContainer_v1.h"
#include "GraphConcepts.h"

namespace sag::tic_tac_toe {

class TicTacToeGraphContainer;
class TicTacToeRules;
class TicTacToeGraphContainer;

struct Graph {
	using state = unsigned short;  // interpret(encode) the board as an integer in base-3
	using action = char;           // an action is a number between 1 and 9 (3x3 board)
	using container = TicTacToeGraphContainer;
	using rules = TicTacToeRules;
	using printer = TicTacToeGraphContainer;
};

const size_t BoardSize = 9;
using Board = std::array<Graph::state, BoardSize>;  // board is of size 3x3 with entries 0=empty, 1=player, 2=opponent

class TicTacToeRules {
 public:
	static auto list_roots() -> std::vector<Graph::state> { return {0}; }
	static auto list_actions(Graph::state state) -> std::vector<Graph::action>;
	static auto list_edges(Graph::state state, Graph::action action) -> std::vector<ActionEdge<Graph::state>>;
	static auto score(Graph::state state) -> Score;

	static auto decode(Graph::state state_id) -> Board;
	static auto encode(const Board& board) -> Graph::state;
	static auto to_string(const Board& board, bool line_break) -> std::string;

 private:
	// tic tac toe is a very simple game - no member fields, no caches, all methods are static
	static auto invert(Board board) -> Board;
	static auto opponent_has_won(const Board& board) -> bool;
};

static_assert(RulesEngine<TicTacToeRules, Graph::state, Graph::action>);

class TicTacToeGraphContainer : public DefaultGraphContainer_v1<Graph::state, Graph::action> {
 public:
	TicTacToeGraphContainer() : DefaultGraphContainer_v1<Graph::state, Graph::action>(TicTacToeRules()) {}

	[[nodiscard]] static auto to_string(Graph::state state) -> std::string;
	[[nodiscard]] static auto to_string_formatted(Graph::state state) -> std::string;
	[[nodiscard]] static auto to_string(Graph::state state, Graph::action action) -> std::string;
	[[nodiscard]] static auto to_string_formatted(Graph::state state, Graph::action action) -> std::string;
};

static_assert(sag::Graph<Graph>);

}  // namespace sag::tic_tac_toe
