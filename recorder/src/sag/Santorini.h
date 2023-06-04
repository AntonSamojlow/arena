#pragma once

#include <array>
#include <bitset>
#include <functional>
#include <stdexcept>

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
	// 		// compute ceil(log_2(array_size())) for array sizes for rows and cols from the interval [2,5]
	// 		case 4: return 10;
	// 		case 6: return 14;
	// 		case 8: return 19;
	// 		case 9: return 21;
	// 		case 10: return 24;
	// 		case 12: return 28;
	// 		case 15: return 35;
	// 		case 16: return 38;
	// 		case 20: return 47;
	// 		case 24: return 56;
	// 		case 25: return 59;
	// 		default: return 0;
	// 	}
	// }
};
// NOLINTEND(*magic-numbers)

struct Postion {
	char row;
	char col;
};

template <Dimensions dim>
class Board {
	std::array<BoardState, dim.array_size()> data_;

	friend auto operator<=>(const Board&, const Board&) = default;

 public:
	Board() = default;
	explicit Board(std::array<BoardState, dim.array_size()> data) : data_(std::move(data)) {}
	[[nodiscard]] auto at(Postion position) const -> BoardState { return data_[position.col + dim.cols * position.row]; }
	[[nodiscard]] auto underlying_array() const -> std::array<BoardState, dim.array_size()> const& { return data_; }
};

template <Dimensions dim>
struct State {
	State() = default;

	State(unsigned long long encoded_board,
		std::array<Postion, dim.player_unit_count> units_player,
		std::array<Postion, dim.player_unit_count> units_opponent)
			: units_player(std::move(units_player)),
				units_opponent(std::move(units_opponent)),
				encoded_board_(encoded_board) {}

	State(Board<dim> board,
		std::array<Postion, dim.player_unit_count> units_player,
		std::array<Postion, dim.player_unit_count> units_opponent)
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

	std::array<Postion, dim.player_unit_count> units_player;
	std::array<Postion, dim.player_unit_count> units_opponent;

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
class Container;

template <Dimensions dim>
class Rules;

template <Dimensions dim>
struct Graph {
	using state = State<dim>;
	using action = Action;
	using container = Container<dim>;
	using rules = Rules<dim>;
	using printer = Container<dim>;
};

template <Dimensions dim>
class Container {};

template <Dimensions dim>
class Rules {
	[[nodiscard]] auto list_roots() const -> std::vector<State<dim>>;
	[[nodiscard]] auto list_actions(State<dim> state) const -> std::vector<Action>;
	[[nodiscard]] auto list_edges(State<dim> state, Action action) const -> std::vector<sag::ActionEdge<State<dim>>>;
	[[nodiscard]] auto score(State<dim> state) const -> tools::Score;
};

}  // namespace sag::santorini

namespace std {

template <sag::santorini::Dimensions dim>
// NOLINTNEXTLINE(cert-dcl58-cpp)
struct std::hash<sag::santorini::State<dim>> {
	auto operator()(sag::santorini::State<dim> const& state) const noexcept -> std::size_t { return state.hash(); }
};
}  // namespace std
