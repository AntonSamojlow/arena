#pragma once

#include <fmt/format.h>

#include <algorithm>
#include <array>
#include <bitset>
#include <functional>
#include <numeric>
#include <stdexcept>
#include <string>

#include "sag/DefaultGraphContainer_v1.h"
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

	auto to_string() -> std::string { return fmt::format("{},{}", std::to_string(row), std::to_string(col)); }
};

template <Dimensions dim>
class Board {
	std::array<BoardState, dim.array_size()> data_{};

	friend auto operator<=>(const Board&, const Board&) = default;

 public:
	Board() {
		for (size_t i = 0; i < dim.array_size(); ++i)
			data_[i] = BoardState::Empty;
	}

	explicit Board(std::array<BoardState, dim.array_size()> data) : data_(std::move(data)) {}
	[[nodiscard]] auto at(Position position) const -> BoardState { return data_[position.col + dim.cols * position.row]; }
	[[nodiscard]] auto increment(Position position) -> void {
		BoardState& current = data_[position.col + dim.cols * position.row];
		if (current == BoardState::Closed)  // sanity check -should never happen
			throw std::logic_error("Can not increment a closed position!");
		current = static_cast<BoardState>(static_cast<unsigned char>(current) + 1);
	}
	[[nodiscard]] auto underlying_array() const -> std::array<BoardState, dim.array_size()> const& { return data_; }
};

template <Dimensions dim>
struct State {
	State() = default;

	State(std::bitset<dim.byte_size()> encoded_board,
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
		size_t hash = std::hash<std::bitset<dim.byte_size()>>()(encoded_board_);
		tools::hash_combine(hash, units_player);
		tools::hash_combine(hash, units_opponent);
		return hash;
	}

	std::array<Position, dim.player_unit_count> units_player;
	std::array<Position, dim.player_unit_count> units_opponent;

 private:
	std::bitset<dim.byte_size()> encoded_board_;

	[[nodiscard]] static auto encode(Board<dim> board) -> std::bitset<dim.byte_size()> {
		unsigned long long result = 0;
		auto const& board_array = board.underlying_array();
		for (size_t k = 0; k < board_array.size(); ++k) {
			result += static_cast<unsigned long long>(board_array[k]) *
								static_cast<size_t>(pow(static_cast<double>(BoardState::VALUE_COUNT), static_cast<double>(k)));
		}
		return result;
	}
};

struct Action {
	char unit_nr;
	Position move_location;
	Position build_location;
	friend auto operator<=>(const Action&, const Action&) = default;
};

template <Dimensions dim>
class Rules {
 public:
	Rules() = default;

	/// concept RulesEngine:

	[[nodiscard]] auto list_roots() const -> std::vector<State<dim>> {
		std::vector<State<dim>> result;
		Board<dim> empty_board;

		std::vector<std::array<Position, dim.player_unit_count>> unit_combinations = recurse_unit_combinations(0, 0);
		for (const std::array<Position, dim.player_unit_count>& player_units : unit_combinations) {
			for (const std::array<Position, dim.player_unit_count>& opponent_units : unit_combinations) {
				if (std::ranges::any_of(player_units, [&opponent_units](const Position& player_unit) {
							return opponent_units.end() != std::ranges::find(opponent_units, player_unit);
						}))
					continue;
				result.emplace_back(empty_board, player_units, opponent_units);
			}
		}
		return result;
	}

	[[nodiscard]] auto list_actions(State<dim> state) const -> std::vector<Action> {
		// skip if state is terminal
		if (score(state).value() == -1.0F || score(state).value() == 1.0F)
			return {};

		std::vector<Action> result;
		Board<dim> const board = get_board(state);

		auto is_occupied = [&state](Position const pos) {
			return std::ranges::find(state.units_player, pos) != state.units_player.end() ||
						 std::ranges::find(state.units_opponent, pos) != state.units_opponent.end();
		};

		for (size_t unit_nr = 0; unit_nr < state.units_player.size(); ++unit_nr) {
			Position const start_from = state.units_player[unit_nr];

			// loop over moves
			for (Position const move_to : neighborhoods_.at(start_from)) {
				if (is_occupied(move_to) || board.at(move_to) == BoardState::Closed ||
						static_cast<unsigned char>(board.at(move_to)) > 1 + static_cast<unsigned char>(board.at(start_from)))
					continue;

				// check valid builds after move
				for (Position const build_at : neighborhoods_.at(move_to)) {
					if (build_at != start_from  // it is always possible to build where moved-from
							&& (is_occupied(build_at) || board.at(build_at) == BoardState::Closed))
						continue;

					result.emplace_back(
						Action{.unit_nr = static_cast<char>(unit_nr), .move_location = move_to, .build_location = build_at});
				}
			}
		}
		return result;
	}

	[[nodiscard]] auto list_edges(State<dim> state, Action action) const -> std::vector<sag::ActionEdge<State<dim>>> {
		return {ActionEdge<State<dim>>(1.0, apply_move(state, action))};
	}

	[[nodiscard]] auto score(State<dim> state) const -> tools::Score {
		Board<dim> const board = get_board(state);
		if (std::ranges::any_of(
					state.units_opponent, [&board](Position unit) { return board.at(unit) == BoardState::Goal; }))
			return tools::Score(-1.0F);

		if (std::ranges::any_of(state.units_player, [&board](Position unit) { return board.at(unit) == BoardState::Goal; }))
			return tools::Score(1.0F);

		return tools::Score{0.0F};
	}

	/// concept VertexPrinter:

	[[nodiscard]] auto to_string(State<dim> state) const -> std::string {
		Board<dim> const board = get_board(state);
		std::array<BoardState, dim.array_size()> const board_data = board.underlying_array();

		int const turn_nr =
			std::accumulate(board_data.begin(), board_data.end(), 0, [](int val, BoardState board_state) -> int {
				return val += static_cast<int>(board_state);
			});
		bool const starting_player_turn = turn_nr % 2 == 0;

		// the display symbol per player is fixed
		auto player_symbol = [starting_player_turn, &state](Position pos) -> char {
			if (std::ranges::find(state.units_opponent, pos) != state.units_opponent.end())
				return starting_player_turn ? 'o' : 'x';
			if (std::ranges::find(state.units_player, pos) != state.units_player.end())
				return starting_player_turn ? 'x' : 'o';
			return ' ';
		};

		std::string result = "[";
		char last_row = 0;
		for (Position pos : sorted_positions_) {
			if (last_row != pos.row) {
				result.append("][");
				last_row = pos.row;
			}
			result.append(' ' + std::to_string(static_cast<unsigned char>(board.at(pos))) + player_symbol(pos));
		}

		result.append("]");
		return result;
	}

	[[nodiscard]] auto to_string(State<dim> state, Action action) const -> std::string {
		return fmt::format("({}->{}, build {}) on {}",
			state.units_player[action.unit_nr].to_string(),
			action.move_location.to_string(),
			action.build_location.to_string(),
			to_string(state));
	}

 private:
	std::map<Position, std::vector<Position>> const neighborhoods_ = create_neighborhoods();
	std::vector<Position> const sorted_positions_ = create_sorted_positions();

	// get the board for the given state, possibly from cache
	auto get_board(State<dim> state) const -> Board<dim> { return state.create_board(); }

	auto apply_move(State<dim> state, Action action) const -> State<dim> {
		Board<dim> board = get_board(state);
		state.units_player[action.unit_nr] = action.move_location;
		board.increment(action.build_location);
		std::swap(state.units_player, state.units_opponent);
		return State<dim>{board, state.units_player, state.units_opponent};
	}

	auto recurse_unit_combinations(size_t unit_index, size_t position_index) const
		-> std::vector<std::array<Position, dim.player_unit_count>> {
		std::vector<std::array<Position, dim.player_unit_count>> result;

		size_t remaining_units = dim.player_unit_count - (unit_index + 1);

		for (size_t i = position_index; i < sorted_positions_.size() - remaining_units; i++) {
			std::vector<std::array<Position, dim.player_unit_count>> recurse_result =
				remaining_units > 0
					? recurse_unit_combinations(unit_index + 1, i + 1)
					: std::vector<std::array<Position, dim.player_unit_count>>({std::array<Position, dim.player_unit_count>()});

			for (std::array<Position, dim.player_unit_count>& next_level : recurse_result) {
				next_level[unit_index] = sorted_positions_[i];
				result.push_back(next_level);
			}
		}
		return result;
	}

	auto static create_sorted_positions() -> std::vector<Position> {
		std::vector<Position> result;
		for (char row = 0; row < dim.rows; ++row) {
			for (char col = 0; col < dim.cols; ++col) {
				result.emplace_back(Position{.row = row, .col = col});
			}
		}
		return result;
	}

	auto static create_neighborhoods() -> std::map<Position, std::vector<Position>> {
		std::map<Position, std::vector<Position>> result;
		for (Position const pos : create_sorted_positions()) {
			std::vector<Position> neighbors;
			neighbors.reserve(3);

			for (char row_offset = -1; row_offset < 2; ++row_offset) {
				for (char col_offset = -1; col_offset < 2; ++col_offset) {
					Position const neighbor{static_cast<char>(pos.row + row_offset), static_cast<char>(pos.col + col_offset)};
					if (neighbor != pos && neighbor.row >= 0 && neighbor.row < dim.rows && neighbor.col >= 0 &&
							neighbor.col < dim.cols) {
						neighbors.emplace_back(neighbor);
					}
				}
			}
			result[pos] = neighbors;
		}
		return result;
	}
};

template <Dimensions dim>
struct Container : public DefaultGraphContainer_v1<State<dim>, Action> {
	Container() : DefaultGraphContainer_v1<State<dim>, Action>(Rules<dim>()) {}
};

template <Dimensions dim>
struct Graph {
	using state = State<dim>;
	using action = Action;
	using container = Container<dim>;
	using rules = Rules<dim>;
	using printer = Rules<dim>;  // resuse rules as printer, thereby sharing caching of `get_board`
};

}  // namespace sag::santorini

namespace std {

template <sag::santorini::Dimensions dim>
// NOLINTNEXTLINE(cert-dcl58-cpp)
struct hash<sag::santorini::State<dim>> {
	auto operator()(sag::santorini::State<dim> const& state) const noexcept -> std::size_t { return state.hash(); }
};

template <>
struct hash<sag::santorini::Position> {
	auto operator()(sag::santorini::Position const& pos) const noexcept -> std::size_t {
		size_t hash = 0;
		tools::hash_combine(hash, pos.row);
		tools::hash_combine(hash, pos.col);
		return hash;
	}
};

template <>
struct hash<sag::santorini::Action> {
	auto operator()(sag::santorini::Action const& action) const noexcept -> std::size_t {
		size_t hash = 0;
		tools::hash_combine(hash, action.unit_nr);
		tools::hash_combine(hash, action.build_location);
		tools::hash_combine(hash, action.move_location);
		return hash;
	}
};

}  // namespace std
