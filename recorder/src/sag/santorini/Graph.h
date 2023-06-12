#pragma once

#include <fmt/format.h>

#include <algorithm>
#include <bitset>
#include <cassert>
#include <concepts>
#include <functional>
#include <numeric>
#include <ranges>
#include <stdexcept>

#include "Santorini.h"
#include "sag/DefaultGraphContainer_v1.h"
#include "sag/santorini/Santorini.h"
#include "tools/Hashing.h"

namespace sag::santorini {
template <Dimensions dim>
struct State {
	State() = default;
	State(std::array<Position, dim.player_unit_count> player_units,
		std::array<Position, dim.player_unit_count> opponent_units,
		unsigned long long encoded_board_base5)
			: units_player(std::move(player_units)),
				units_opponent(std::move(opponent_units)),
				board_base5(encoded_board_base5) {}

	State(std::array<Position, dim.player_unit_count> player_units,
		std::array<Position, dim.player_unit_count> opponent_units,
		Board<dim> board)
			: State(player_units, opponent_units, board.encode_base5()) {}

	friend auto operator<=>(const State&, const State&) = default;

	[[nodiscard]] auto hash() const -> size_t {
		size_t hash = std::hash<std::bitset<dim.encoded_board_bitcount()>>()(board_base5);
		tools::hash_combine(hash, units_player);
		tools::hash_combine(hash, units_opponent);
		return hash;
	}

	std::array<Position, dim.player_unit_count> units_player;
	std::array<Position, dim.player_unit_count> units_opponent;
	std::bitset<dim.encoded_board_bitcount()> board_base5;
};

struct Action {
	unsigned char unit_nr;
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

		auto const unit_combinations = recurse_unit_combinations(0, 0);
		for (auto const& player_units : unit_combinations) {
			for (auto const& opponent_units : unit_combinations) {
				if (std::ranges::any_of(player_units, [&opponent_units](const Position& player_unit) {
							return opponent_units.end() != std::ranges::find(opponent_units, player_unit);
						}))
					continue;
				result.emplace_back(player_units, opponent_units, empty_board);
			}
		}
		return result;
	}

	[[nodiscard]] auto list_actions(State<dim> state) const -> std::vector<Action> {
		if (opponent_has_won(state))
			return {};

		std::vector<Action> result;
		Board<dim> const board = get_board(state);

		auto is_free = [&state, &board](Position pos) {
			return std::ranges::find(state.units_player, pos) == state.units_player.end() &&
						 std::ranges::find(state.units_opponent, pos) == state.units_opponent.end() &&
						 board.at(pos) != BoardState::Closed;
		};

		for (size_t unit_nr = 0; unit_nr < state.units_player.size(); ++unit_nr) {
			Position const start_from = state.units_player[unit_nr];

			auto valid_moves =
				neighborhoods_.at(start_from) | std::views::filter([&board, &is_free, &start_from](Position move_to) {
					return is_free(move_to) &&
								 static_cast<unsigned char>(board.at(move_to)) < 2 + static_cast<unsigned char>(board.at(start_from));
				});

			auto valid_builds = [&start_from, &is_free, this](Position move_to) {
				return neighborhoods_.at(move_to) | std::views::filter([&is_free, &start_from](Position build_at) {
					return start_from != build_at && is_free(build_at);
				});
			};

			for (Position move_to : valid_moves) {
				for (Position build_at : valid_builds(move_to)) {
					result.emplace_back(Action{
						.unit_nr = static_cast<unsigned char>(unit_nr), .move_location = move_to, .build_location = build_at});
				}
			}
		}
		return result;
	}

	[[nodiscard]] auto list_edges(State<dim> state, Action action) const -> std::vector<sag::ActionEdge<State<dim>>> {
		return {ActionEdge<State<dim>>(1.0, apply_move(state, action))};
	}

	[[nodiscard]] auto score(State<dim> state) const -> tools::Score {
		return tools::Score{list_actions(state).empty() ? -1.0F : 0.0F};
	}

	/// concept VertexPrinter:

	[[nodiscard]] auto to_string(State<dim> state) const -> std::string {
		Board<dim> const board = get_board(state);
		auto const board_data = board.underlying_array();

		int const turn_nr = std::accumulate(board_data.begin(), board_data.end(), 0, [](int val, BoardState board_state) {
			return val += static_cast<int>(board_state);
		});
		bool const starting_player_turn = turn_nr % 2 == 0;

		// the display symbol per player is fixed
		auto player_symbol = [starting_player_turn, &state](Position pos) {
			if (std::ranges::find(state.units_opponent, pos) != state.units_opponent.end())
				return starting_player_turn ? 'o' : 'x';
			if (std::ranges::find(state.units_player, pos) != state.units_player.end())
				return starting_player_turn ? 'x' : 'o';
			return ' ';
		};

		std::string result = "[";
		unsigned char last_row = 0;
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
	auto get_board(State<dim> state) const -> Board<dim> {
		return Board<dim>::decode_base5(state.board_base5.to_ullong());
	}

	auto opponent_has_won(State<dim> state) const -> bool {
		Board<dim> const board = get_board(state);
		auto unit_has_won = [&board](Position unit) {
			return board.at(unit) == BoardState::Goal;
		};

		// can not be reached from walking down a graph (state gets inverted, to refelct the *active* players view)
		assert(std::ranges::none_of(state.units_player, unit_has_won));

		return std::ranges::any_of(state.units_opponent, unit_has_won);
	}

	auto apply_move(State<dim> state, Action action) const -> State<dim> {
		Board<dim> board = get_board(state);
		state.units_player[action.unit_nr] = action.move_location;
		board.increment(action.build_location);
		std::swap(state.units_player, state.units_opponent);
		return {state.units_player, state.units_opponent, board};
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
		for (unsigned char row = 0; row < dim.rows; ++row) {
			for (unsigned char col = 0; col < dim.cols; ++col) {
				result.emplace_back(Position{.row = row, .col = col});
			}
		}
		return result;
	}

	auto static create_neighborhood_at(Position pos) -> std::vector<Position> {
		std::vector<Position> neighbors;
		neighbors.reserve(3);

		for (char row_offset = -1; row_offset < 2; ++row_offset) {
			char row = static_cast<char>(pos.row + row_offset);
			if (row < 0 || row >= static_cast<char>(dim.rows))
				continue;

			for (char col_offset = -1; col_offset < 2; ++col_offset) {
				char col = static_cast<char>(pos.col + col_offset);
				if (col < 0 || col >= static_cast<char>(dim.cols))
					continue;

				Position const neighbor{static_cast<unsigned char>(row), static_cast<unsigned char>(col)};
				if (neighbor != pos)
					neighbors.emplace_back(neighbor);
			}
		}
		return neighbors;
	}

	auto static create_neighborhoods() -> std::map<Position, std::vector<Position>> {
		std::map<Position, std::vector<Position>> result;
		for (Position const pos : create_sorted_positions()) {
			result[pos] = create_neighborhood_at(pos);
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
