#pragma once

#include <fmt/format.h>

#include <array>
#include <cassert>
#include <map>
#include <string>
#include <vector>

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
	unsigned char row;
	unsigned char col;
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
		assert(current != BoardState::Closed);  // sanity check
		current = static_cast<BoardState>(static_cast<unsigned char>(current) + 1);
	}

	[[nodiscard]] auto underlying_array() const -> std::array<BoardState, dim.position_count()> const& { return data_; }

	[[nodiscard]] auto static decode_base5(unsigned long long encoded_value) -> Board<dim> {
		std::array<BoardState, dim.position_count()> board_data;
		for (size_t k = dim.position_count(); k > 0; --k) {
			auto const factor =
				static_cast<unsigned long long>(pow(static_cast<double>(BoardState::VALUE_COUNT), static_cast<double>(k - 1)));
			auto const entry = static_cast<unsigned char>(encoded_value / factor);
			board_data[k - 1] = static_cast<BoardState>(entry);
			encoded_value -= entry * factor;
		}
		return Board<dim>{board_data};
	}

	[[nodiscard]] auto encode_base5() const -> unsigned long long {
		unsigned long long result = 0;
		for (size_t k = 0; k < data_.size(); ++k) {
			result += static_cast<unsigned long long>(data_[k]) *
								static_cast<size_t>(pow(static_cast<double>(BoardState::VALUE_COUNT), static_cast<double>(k)));
		}
		return result;
	}
};

}  // namespace sag::santorini
