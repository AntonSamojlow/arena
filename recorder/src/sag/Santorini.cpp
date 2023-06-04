#include "Santorini.h"
#include "sag/GraphConcepts.h"

namespace sag::santorini {

namespace {
constexpr Dimensions small{.rows = 2, .cols = 2, .player_unit_count = 1};

static_assert(std::regular<Board<small>>);
static_assert(std::regular<State<small>>);

static_assert(sag::Vertices<State<small>, Action>);
static_assert(sag::RulesEngine<Rules<small>,State<small>,Action>);


}  // namespace

}  // namespace sag::santorini
