#pragma once

#include <vcruntime.h>

#include <tl/expected.hpp>
#include <vector>

#include "Storage.h"
#include "sag/Failure.h"
#include "sag/GraphConcepts.h"

namespace sag::match {
template <typename S, typename A>
	requires sag::Vertices<S, A>
class MemoryStorage {
	struct Entry {
		Entry() = default;
		Entry(Match<S, A> const& match_value, std::string_view extra_data_view)
				: match(match_value), extra_data(extra_data_view) {}
		Entry(Match<S, A>&& match_value, std::string_view extra_data_view)
				: match{std::move(match_value)}, extra_data{extra_data_view} {}

		Match<S, A> match;
		std::string extra_data;
	};

 public:
	auto add(Match<S, A> match, std::string_view extra_data) -> tl::expected<void, Failure> {
		data_.emplace_back(match, extra_data);
		return {};
	}

	[[nodiscard]] auto size() const -> size_t { return data_.size(); }

 private:
	std::vector<MemoryStorage<S, A>::Entry> data_ = {};
};

static_assert(Storage<MemoryStorage<int, int>, int, int>);

}  // namespace sag::match
