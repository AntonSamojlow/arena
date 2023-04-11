#include "TicTacToe.h"

#include <fmt/format.h>

#include <vector>

namespace sag::tic_tac_toe {
auto Rules::list_actions(Graph::state state) -> std::vector<Graph::action> {
	Board const board = decode(state);

	// skip if opponent has won
	if (opponent_has_won(board))
		return {};

	std::vector<Graph::action> actions;

	for (size_t i = 0; i < BoardSize; i++) {
		if (board[i] == 0)
			actions.emplace_back(static_cast<Graph::action>(i));
	}
	return actions;
}

// NOLINTNEXTLINE (bugprone-easily-swappable-parameters)
auto Rules::list_edges(Graph::state state, Graph::action action) -> std::vector<ActionEdge<Graph::state>> {
	Board board = decode(state);
	++board[static_cast<size_t>(action)];
	return {ActionEdge<Graph::state>(1.0, encode(invert(board)))};
}

auto Rules::score(Graph::state state) -> tools::Score {
	return opponent_has_won(decode(state)) ? tools::Score(-1.0F) : tools::Score(0.0F);
}

auto Rules::encode(const Board& board) -> Graph::state {
	Graph::state value = 0;

	for (size_t i = 0; i < BoardSize; i++) {
		value += static_cast<Graph::state>(board[i] * static_cast<Graph::state>(std::pow(3, i)));
	}
	return value;
}

auto Rules::decode(Graph::state state_id) -> Board {
	Board board{};
	for (size_t i = BoardSize; i > 0; i--) {
		board[i - 1] = static_cast<unsigned char>(state_id / std::pow(3, i - 1));
		state_id -= static_cast<Graph::state>(board[i - 1] * std::pow(3, i - 1));
	}
	return board;
}

auto Rules::invert(Board board) -> Board {
	for (size_t i = 0; i < BoardSize; i++) {
		if (board[i] > 0)
			board[i] = 1 + (board[i] % 2);
	}
	return board;
}

// NOLINTBEGIN(*-magic-numbers)
auto Rules::opponent_has_won(const Board& board) -> bool {
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

auto Rules::to_string(const Board& board, bool line_break) -> std::string {
	int empty_spaces = 0;
	for (auto const& entry : board) {
		if (entry == 0)
			++empty_spaces;
	}
	bool const starting_player_turn = empty_spaces % 2 == 0;

	// the display symbol is fixed
	auto display_symbol = [&starting_player_turn](unsigned short board_value) {
		if (board_value == 1)
			return starting_player_turn ? 'o' : 'x';
		if (board_value == 2)
			return starting_player_turn ? 'x' : 'o';
		return ' ';
	};

	std::string result;
	for (size_t i = 0; i < 3; i++) {
		for (size_t j = 0; j < 3; j++) {
			unsigned short const value = board[i * 3 + j];
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

auto Container::to_string(Graph::state state) -> std::string {
	return Rules::to_string(Rules::decode(state), false);
}

auto Container::to_string(Graph::state state, Graph::action action) -> std::string {
	return fmt::format("action-{} at: {})", action, to_string(state));
}

auto Container::to_string_formatted(Graph::state state) -> std::string {
	return Rules::to_string(Rules::decode(state), true);
}

auto Container::to_string_formatted(Graph::state state, Graph::action action) -> std::string {
	return fmt::format("action-{} at::\n{})", action, to_string(state));
}

}  // namespace sag::tic_tac_toe
