#pragma once

#include "Storage.h"
#include "sag/Failure.h"
#include "sag/GraphConcepts.h"

namespace sag::match {
template <typename S, typename A>
	requires sag::Vertices<S, A>
class PostgresStorage {

 public:
	auto add(Match<S, A> match, std::string_view extra_data) -> tl::expected<void, Failure> {
		return {};
	}

};

static_assert(Storage<PostgresStorage<int, int>, int, int>);

}  // namespace sag::match
