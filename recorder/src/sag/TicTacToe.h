#pragma once
#include <array>

#include "DefaultGraphContainer_v1.h"
#include "GraphConcepts.h"

namespace sag::tic_tac_toe {

const size_t BoardSize = 9;

using Board = std::array<unsigned char, BoardSize>;  // board is of size 3x3 with entries 0=empty, 1=player, 2=opponent
using StateId = unsigned short;                      // interpret(encode) the board as an integer in base-3
using ActionId = char;                               // an action is a number between 1 and 9 (3x3 board)

class TicTacToeRules {
 public:
	static auto list_roots() -> std::vector<StateId> { return {0}; }
	static auto list_actions(StateId state) -> std::vector<ActionId>;
	static auto list_edges(StateId state, ActionId action) -> std::vector<ActionEdge<StateId>>;
	static auto score(StateId state) -> Score;

	static auto decode(StateId state_id) -> Board;
	static auto encode(const Board& board) -> StateId;
	static auto to_string(const Board& board, bool line_break) -> std::string;

 private:
	// tic tac toe is a very simple game - no member fields, no caches, all methods are static
	static auto invert(Board board) -> Board;
	static auto opponent_has_won(const Board& board) -> bool;
};

static_assert(RulesEngine<TicTacToeRules, StateId, ActionId>);

class TicTacToeGraph : public DefaultGraphContainer_v1<StateId, ActionId> {
 public:
	TicTacToeGraph() : DefaultGraphContainer_v1<StateId, ActionId>(TicTacToeRules()) {}

	[[nodiscard]] static auto stringify(StateId state) -> std::string;
	[[nodiscard]] static auto stringify_formatted(StateId state) -> std::string;
	[[nodiscard]] static auto stringify(StateId state, ActionId action) -> std::string;
	[[nodiscard]] static auto stringify_formatted(StateId state, ActionId action) -> std::string;
};

static_assert(GraphContainer<TicTacToeGraph, StateId, ActionId>);
static_assert(VertexStringifier<TicTacToeGraph, StateId, ActionId>);

}  // namespace sag::tic_tac_toe
