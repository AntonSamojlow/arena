#include "TicTacToe.h"

#include <fmt/format.h>

#include <vector>

#include "TicTacToe.h"

namespace sag::tic_tac_toe {
auto TicTacToeRules::list_actions(StateId state) -> std::vector<ActionId> {
	Board const board = decode(state);

	// skip if opponent has won
	if (opponent_has_won(board))
		return {};

	std::vector<ActionId> actions;

	for (size_t i = 0; i < BoardSize; i++) {
		if (board[i] == 0)
			actions.emplace_back(static_cast<ActionId>(i));
	}
	return actions;
}

// NOLINTNEXTLINE (bugprone-easily-swappable-parameters)
auto TicTacToeRules::list_edges(StateId state, ActionId action) -> std::vector<ActionEdge<StateId>> {
	Board board = decode(state);
	++board[static_cast<size_t>(action)];
	return {ActionEdge<StateId>(1.0, encode(invert(board)))};
}

auto TicTacToeRules::score(StateId state) -> double {
	return opponent_has_won(decode(state)) ? -1.0 : 0.0;
}

auto TicTacToeRules::encode(const Board& board) -> StateId {
	StateId value = 0;

	for (size_t i = 0; i < BoardSize; i++) {
		value += static_cast<StateId>(board[i] * static_cast<StateId>(std::pow(3, i)));
	}
	return value;
}

auto TicTacToeRules::decode(StateId state_id) -> Board {
	Board board{};
	for (size_t i = BoardSize; i > 0; i--) {
		board[i - 1] = static_cast<unsigned char>(state_id / std::pow(3, i - 1));
		state_id -= static_cast<StateId>(board[i - 1] * std::pow(3, i - 1));
	}
	return board;
}

auto TicTacToeRules::invert(Board board) -> Board {
	for (size_t i = 0; i < BoardSize; i++) {
		if (board[i] > 0)
			board[i] = 1 + (board[i] % 2);
	}
	return board;
}

// NOLINTBEGIN(*-magic-numbers)
auto TicTacToeRules::opponent_has_won(const Board& board) -> bool {
	if (board[0] == 2) {
		if (2 == board[1] && 2 == board[2])
			return true;
		if (2 == board[3] && 2 == board[6])
			return true;
		if (2 == board[4] && 2 == board[8])
			return true;
	}

	if (2 == board[1] && 2 == board[4] && 2 == board[7])
		return true;
	if (2 == board[2] && 2 == board[5] && 2 == board[8])
		return true;
	if (2 == board[3] && 2 == board[4] && 2 == board[5])
		return true;
	if (2 == board[6] && 2 == board[7] && 2 == board[8])
		return true;
	if (2 == board[6] && 2 == board[4] && 2 == board[2])
		return true;

	return false;
}
// NOLINTEND(*-magic-numbers)

auto TicTacToeRules::to_string(const Board& board, bool line_break) -> std::string {
	int empty_spaces = 0;
	for (auto const& entry : board) {
		if (entry == 0)
			++empty_spaces;
	}
	bool const starting_player_turn = empty_spaces % 2 == 0;

	// the display symbol is fixed
	auto display_symbol = [&starting_player_turn](unsigned char board_value) {
		if (board_value == 1)
			return starting_player_turn ? 'o' : 'x';
		if (board_value == 2)
			return starting_player_turn ? 'x' : 'o';
		return ' ';
	};

	std::string result;
	for (size_t i = 0; i < 3; i++) {
		for (size_t j = 0; j < 3; j++) {
			unsigned char const value = board[i * 3 + j];
			result.push_back('|');
			result.push_back(display_symbol(value));
		}
		if (line_break && i < 2) {
			result += "|\n";
		}
	}
	result.push_back('|');
	return result;
}

auto TicTacToeGraph::stringify(StateId state) const -> std::string {
	const auto& rules_engine = get_rules_engine();
	return rules_engine.to_string(rules_engine.decode(state), false);
}

auto TicTacToeGraph::stringify(StateId state, ActionId action) const -> std::string {
	return fmt::format("action-{} at: {})", action, stringify(state));
}

auto TicTacToeGraph::stringify_formatted(StateId state) const -> std::string {
	const auto& rules_engine = get_rules_engine();
	return rules_engine.to_string(rules_engine.decode(state), true);
}

auto TicTacToeGraph::stringify_formatted(StateId state, ActionId action) const -> std::string {
	return fmt::format("action-{} at::\n{})", action, stringify(state));
}

}  // namespace sag::tic_tac_toe