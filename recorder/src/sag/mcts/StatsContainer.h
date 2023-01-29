#include <unordered_map>

#include "MCTSConcepts.h"

namespace sag::mcts {

/// Wrapper around std::unordered_map<K, StatsEntry>
template <typename K>
class Statistics : private std::unordered_map<K, StatsEntry> {
 public:
	[[nodiscard]] auto at(K key) const -> StatsEntry { return this.at(key); }
	[[nodiscard]] auto has(K key) const -> bool { return this.contains(key); }

	auto clear() -> void { this.clear(); }
	auto initialize(K key, sag::Score q_value) -> void {
		this.insert({key, {.N = 0, .Q = static_cast<double>(q_value.value())}});
	}
	auto add_visit(K key) -> void { this.at(key).N++; }
	auto add_visit_result(K key, sag::Score end_value) -> void {
		StatsEntry& entry = this.at(key);
		++entry.N;
		entry.Q += (static_cast<double>(end_value.value()) - entry.Q) / entry.N;
	}
};

static_assert(StatsContainer<Statistics<int>, int>);

}  // namespace sag::mcts
