#pragma once
#include <unordered_map>

#include "MCTSConcepts.h"

namespace sag::mcts {

/// Wrapper around std::unordered_map<K, StatsEntry>
template <typename K>
class Statistics {
 public:
	[[nodiscard]] auto at(K key) const -> StatsEntry { return data_.at(key); }
	[[nodiscard]] auto has(K key) const -> bool { return data_.contains(key); }
	[[nodiscard]] auto size() const -> size_t { return data_.size(); }

	auto clear() -> void { this->clear(); }
	auto initialize(K key, sag::Score q_value) -> void { data_.insert({key, {.N = 0, .Q = q_value.value()}}); }
	auto add_visit(K key) -> void { data_.at(key).N++; }
	auto add_visit_result(K key, sag::Score end_value) -> void {
		StatsEntry& entry = data_.at(key);
		++entry.N;
		entry.Q += (end_value.value() - entry.Q) / static_cast<float>(entry.N);
	}

	friend auto operator<=>(const Statistics<K>&, const Statistics<K>&) = default;

 private:
	std::unordered_map<K, StatsEntry> data_;
};

static_assert(StatsContainer<Statistics<int>, int>);

}  // namespace sag::mcts
