#pragma once

#include <algorithm>
#include <array>
#include <bitset>
#include <functional>
#include <stdexcept>
#include <vector>

#include "sag/DefaultGraphContainer_v1.h"
#include "sag/GraphConcepts.h"
#include "tools/Hashing.h"

namespace sag::santorini {

enum class BoardState : unsigned char { Empty = 0, First = 1, Second = 2, Goal = 3, Closed = 4, VALUE_COUNT = 5 };

// NOLINTBEGIN(*magic-numbers)
struct Dimensions {
	size_t rows;
	size_t cols;
	size_t player_unit_count;

	[[nodiscard]] constexpr auto array_size() const -> size_t { return rows * cols; }

	// compute integer upper bound of log_2(array_size()), using that log(5)/log(2) ~ 2.322
	[[nodiscard]] constexpr auto byte_size() const -> size_t {
		return 1 + static_cast<size_t>(static_cast<double>(array_size()) * 2.322);
	}

	// [[nodiscard]] constexpr auto byte_size() const -> size_t {
	// 	switch (array_size()) {
	// 		// compute ceil(log_2(array_size())) for array sizes for rows and cols from the interval
	// [2,5] 		case 4: return 10; 		case 6: return 14; 		case 8: return 19; 		case 9: return 21;
	// case 10: return 24; 		case 12: return 28; 		case 15: return 35; 		case 16: return 38;
	// case 20: return 47; 		case 24: return 56; 		case 25: return 59; 		default: return 0;
	// 	}
	// }
};
// NOLINTEND(*magic-numbers)

struct Position {
	char row;
	char col;
	friend auto operator<=>(const Position&, const Position&) = default;
};

template <Dimensions dim>
class Board {
	std::array<BoardState, dim.array_size()> data_;

	friend auto operator<=>(const Board&, const Board&) = default;

 public:
	Board() = default;
	explicit Board(std::array<BoardState, dim.array_size()> data) : data_(std::move(data)) {}
	[[nodiscard]] auto at(Position position) const -> BoardState { return data_[position.col + dim.cols * position.row]; }
	[[nodiscard]] auto underlying_array() const -> std::array<BoardState, dim.array_size()> const& { return data_; }

	[[nodiscard]] auto static create_all_positions() -> std::vector<Position> {
		std::vector<Position> result;
		for (char row = 0; row < dim.rows; ++row) {
			for (char col = 0; col < dim.cols; ++col) {
				result.emplace_back(Position{.row = row, .col = col});
			}
		}
		return result;
	}
};

template <Dimensions dim>
struct State {
	State() = default;

	State(unsigned long long encoded_board,
		std::array<Position, dim.player_unit_count> units_player,
		std::array<Position, dim.player_unit_count> units_opponent)
			: units_player(std::move(units_player)),
				units_opponent(std::move(units_opponent)),
				encoded_board_(encoded_board) {}

	State(Board<dim> board,
		std::array<Position, dim.player_unit_count> units_player,
		std::array<Position, dim.player_unit_count> units_opponent)
			: units_player(std::move(units_player)),
				units_opponent(std::move(units_opponent)),
				encoded_board_(std::move(State<dim>::encode(board))) {}

	[[nodiscard]] auto create_board() const -> Board<dim> {
		std::array<BoardState, dim.array_size()> board_data;
		unsigned long long encoded_value = encoded_board_.to_ullong();
		for (size_t k = dim.array_size(); k > 0; --k) {
			auto const factor =
				static_cast<unsigned long long>(pow(static_cast<double>(BoardState::VALUE_COUNT), static_cast<double>(k - 1)));
			auto const entry = static_cast<unsigned char>(encoded_value / factor);
			board_data[k - 1] = static_cast<BoardState>(entry);
			encoded_value -= entry * factor;
		}
		return Board<dim>{board_data};
	}

	friend auto operator<=>(const State&, const State&) = default;

	[[nodiscard]] auto hash() const -> size_t {
		size_t hash = std::hash(encoded_board_);
		tools::hash_combine(hash, units_player);
		tools::hash_combine(hash, units_opponent);
		return hash;
	}

	std::array<Position, dim.player_unit_count> units_player;
	std::array<Position, dim.player_unit_count> units_opponent;

 private:
	std::bitset<dim.byte_size()> encoded_board_;

	[[nodiscard]] static auto encode(Board<dim> board) -> unsigned long long {
		unsigned long long result = 0;
		auto const& board_array = board.underlying_array();
		for (size_t k = 0; k < board_array.size(); ++k) {
			result += static_cast<unsigned long long>(board_array[k]) *
								static_cast<size_t>(pow(static_cast<double>(BoardState::VALUE_COUNT), static_cast<double>(k)));
		}
		return result;
	}
};

using Action = unsigned char;

template <Dimensions dim>
class Rules {
 public:
	Rules() {
		// compute the neighborhoods
		for (Position const pos : Board<dim>::create_all_positions()) {
			std::vector<Position> neighbors{3};
			for (char row_offset = -1; row_offset < 2; ++row_offset) {
				for (char col_offset = -1; col_offset < 2; ++col_offset) {
					Position const neighbor{static_cast<char>(pos.row + row_offset), static_cast<char>(pos.col + col_offset)};
					if (neighbor != pos && neighbor.row >= 0 && neighbor.row < dim.rows && neighbor.col >= 0 &&
							neighbor.col < dim.cols) {
						neighbors.emplace_back(neighbor);
					}
				}
			}
			neighborhoods_.emplace(pos, neighbors);
		}
	}
	[[nodiscard]] auto list_roots() const -> std::vector<State<dim>> {
		std::vector<State<dim>> results;
		Board<dim> board;
		// generate all unit combinations
		for(size_t player_unit= 0; player_unit < dim.player_unit_count; ++player_unit)
		{
		}

		std::array<Position, dim.player_unit_count> player_units;
		std::array<Position, dim.player_unit_count> opponent_units;
		results.emplace_back(board, player_units, opponent_units);
		return results;
	}

	[[nodiscard]] auto list_actions(State<dim> state) const -> std::vector<Action>;
	[[nodiscard]] auto list_edges(State<dim> state, Action action) const -> std::vector<sag::ActionEdge<State<dim>>>;

	[[nodiscard]] auto score(State<dim> state) const -> tools::Score {
		Board<dim> const board = get_board(state);

		if (list_actions(state).empty() || std::ranges::any_of(state.units_opponent,
																				 [&board](Position unit) { return board.at(unit) == BoardState::Goal; }))
			return tools::Score(-1.0F);

		if (std::ranges::any_of(state.units_player, [&board](Position unit) { return board.at(unit) == BoardState::Goal; }))
			return tools::Score(1.0F);

		return tools::Score{0.0F};
	}

 private:
	std::map<State<dim>, std::vector<State<dim>>> neighborhoods_;

	// get the board for the given state, possibly from cache
	auto get_board(State<dim> state) -> Board<dim> { return state.create_board(); }
};

template <Dimensions dim>
class Container : public DefaultGraphContainer_v1<State<dim>, Action> {
	Container() : DefaultGraphContainer_v1<State<dim>, Action>(Rules<dim>()) {}
};

template <Dimensions dim>
struct Graph {
	using state = State<dim>;
	using action = Action;
	using container = Container<dim>;
	using rules = Rules<dim>;
	using printer = Container<dim>;
};

}  // namespace sag::santorini

namespace std {

template <sag::santorini::Dimensions dim>
// NOLINTNEXTLINE(cert-dcl58-cpp)
struct std::hash<sag::santorini::State<dim>> {
	auto operator()(sag::santorini::State<dim> const& state) const noexcept -> std::size_t { return state.hash(); }
};
}  // namespace std
