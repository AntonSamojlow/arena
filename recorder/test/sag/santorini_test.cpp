#include "sag/santorini/Santorini.h"

#include <catch2/catch_test_macros.hpp>

using namespace sag::santorini;

namespace {

TEST_CASE("Position test", "[sag, santorini]") {
	Position row1_col1{1, 1};
	Position row1_col2{1, 2};
	Position row2_col1{2, 1};

	// test they are correctly ordered
	CHECK(row1_col1 < row1_col2);
	CHECK(row1_col1 < row2_col1);
	CHECK(row1_col2 < row2_col1);
}

TEST_CASE("Dimension test", "[sag, santorini]") {
	for (size_t row = 2; row <= 10; ++row) {
		for (size_t col = 2; col <= 10; ++col) {
			for (size_t units = 0; units <= 10; ++units) {
				Dimensions dim{.rows = row, .cols = col, .player_unit_count = units};
				size_t bitcount = dim.encoded_board_bitcount();
				size_t expected_bitcount =
					static_cast<size_t>(ceil(log(5) / log(2) * static_cast<double>(dim.rows * dim.cols)));
				CHECK(bitcount == expected_bitcount);
				CHECK(dim.position_count() == dim.rows * dim.cols);
			}
		}
	}
}

}  // namespace
