#include "Santorini.h"

namespace sag::santorini {

namespace {
constexpr Dimensions smallest_dim{.rows = 2, .cols = 2, .player_unit_count = 1};
static_assert(std::regular<Board<smallest_dim>>);
static_assert(std::regular<State<smallest_dim>>);
}  // namespace

}  // namespace sag::santorini
