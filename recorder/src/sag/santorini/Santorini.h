#pragma once

#include <fmt/format.h>

#include <array>
#include <string>

namespace sag::santorini {

enum class BoardState : unsigned char { Empty = 0, First = 1, Second = 2, Goal = 3, Closed = 4, VALUE_COUNT = 5 };

// NOLINTBEGIN(*magic-numbers)
struct Dimensions {
	size_t rows;
	size_t cols;
	size_t player_unit_count;

	[[nodiscard]] constexpr auto position_count() const -> size_t { return rows * cols; }

	/// computes the required number of bits to encode all possible board positions (5^(rows*cols))
	/// as the integer upper bound of log_2(array_size()), using that log(5)/log(2) ~ 2.322
	[[nodiscard]] constexpr auto encoded_board_bitcount() const -> size_t {
		return 1 + static_cast<size_t>(static_cast<double>(position_count()) * 2.322);
	}
};
// NOLINTEND(*magic-numbers)

struct Position {
	char row;
	char col;
	friend auto operator<=>(const Position&, const Position&) = default;

	[[nodiscard]] auto to_string() const -> std::string {
		return fmt::format("{},{}", std::to_string(row), std::to_string(col));
	}
};

template <Dimensions dim>
class Board {
	std::array<BoardState, dim.position_count()> data_{};

	friend auto operator<=>(const Board&, const Board&) = default;

 public:
	Board() {
		for (size_t i = 0; i < dim.position_count(); ++i)
			data_[i] = BoardState::Empty;
	}

	explicit Board(std::array<BoardState, dim.position_count()> data) : data_(std::move(data)) {}

	[[nodiscard]] auto at(Position position) const -> BoardState { return data_[position.col + dim.cols * position.row]; }

	auto increment(Position position) -> void {
		BoardState& current = data_[position.col + dim.cols * position.row];
		if (current == BoardState::Closed)  // sanity check -should never happen
			throw std::logic_error("Can not increment a closed position!");
		current = static_cast<BoardState>(static_cast<unsigned char>(current) + 1);
	}

	[[nodiscard]] auto underlying_array() const -> std::array<BoardState, dim.position_count()> const& { return data_; }
};

}  // namespace sag::santorini
