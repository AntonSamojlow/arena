#pragma once
#include <unordered_map>

#include "sag/GraphConcepts.h"

namespace sag::mcts {

// todo: improve naming
struct StatsEntry {
	int N = 0;
	float Q = 0.0;

	friend auto operator<=>(const StatsEntry&, const StatsEntry&) = default;
};

static_assert(std::regular<StatsEntry>);

// disable until clang-format 16 with 'RequiresExpressionIndentation : OuterScope' is available
// clang-format off

template <typename C, typename Key>
concept StatsContainer = sag::Identifier<Key> && std::regular<C> &&
requires(const C const_container, C container, Key key, tools::Score score) {
	{ const_container.at(key) } -> std::same_as<StatsEntry>;
	{ const_container.has(key) } -> std::same_as<bool>;
	{ const_container.size() } -> std::same_as<size_t>;

	{ container.clear() } -> std::same_as<void>;
	{ container.initialize(key, score) } -> std::same_as<void>;
	{ container.add_visit(key) } -> std::same_as<void>;
	{ container.add_visit_result(key, score) } -> std::same_as<void>;
};

/// Wrapper around std::unordered_map<K, StatsEntry>
template <typename K>
class Statistics {
 public:
	[[nodiscard]] auto at(K key) const -> StatsEntry { return data_.at(key); }
	[[nodiscard]] auto has(K key) const -> bool { return data_.contains(key); }
	[[nodiscard]] auto size() const -> size_t { return data_.size(); }

	auto clear() -> void { data_.clear(); }
	auto initialize(K key, tools::Score q_value) -> void { data_.insert({key, {.N = 0, .Q = q_value.value()}}); }
	auto add_visit(K key) -> void { data_.at(key).N++; }
	auto add_visit_result(K key, tools::Score end_value) -> void {
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
