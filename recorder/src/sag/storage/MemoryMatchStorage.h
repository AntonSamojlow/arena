#pragma once

#include "sag/GraphConcepts.h"
#include "sag/rec/Match.h"

namespace sag::storage {

template <typename S, typename A>
	requires sag::Vertices<S, A>
class MemoryMatchStorage {
	/// Simple type stored on the heap (within a vector)
	struct Entry {
		rec::Match<S, A> match;
		std::string extra_data;

		Entry() = default;
		Entry(rec::Match<S, A> const& match_value, std::string_view extra_data_view)
				: match(match_value), extra_data(extra_data_view) {}
		Entry(rec::Match<S, A>&& match_value, std::string_view extra_data_view)
				: match{std::move(match_value)}, extra_data{extra_data_view} {}
	};

 public:
	auto add(rec::Match<S, A> match, std::string_view extra_data) -> tl::expected<void, tools::Failure> {
		data_.emplace_back(match, extra_data);
		return {};
	}

	[[nodiscard]] auto size() const -> size_t { return data_.size(); }

 private:
	std::vector<MemoryMatchStorage<S, A>::Entry> data_ = {};
};

static_assert(rec::Storage<MemoryMatchStorage<int, int>, int, int>);

}  // namespace sag::storage
