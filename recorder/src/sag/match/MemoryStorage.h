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
		Entry(Match<S, A> const& match_value, std::string const& json_data_value)
				: match(match_value), json_data(json_data_value) {}
		Entry(Match<S, A>&& match_value, std::string&& json_data_value)
				: match{std::move(match_value)}, json_data{std::move(json_data_value)} {}

		Match<S, A> match;
		std::string json_data;
	};

 public:
	auto add(Match<S, A> match, std::string json_data) -> tl::expected<void, Failure> {
		data_.emplace_back(match, json_data);
		return {};
	}

	[[nodiscard]] auto size() const -> size_t { return data_.size(); }

 private:
	std::vector<MemoryStorage<S, A>::Entry> data_ = {};
};

static_assert(Storage<MemoryStorage<int, int>, int, int>);

}  // namespace sag::match
