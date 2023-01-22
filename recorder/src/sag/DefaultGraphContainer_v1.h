#pragma once
#include <algorithm>
#include <memory>
#include <mutex>
#include <random>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "GraphConcepts.h"

namespace sag {

template <typename S, typename A, RulesEngine<S, A> R>
class DefaultGraphContainer_v1 {
 public:
	explicit DefaultGraphContainer_v1(R&& rules_engine) : rules_engine_(rules_engine) {
		roots_ = rules_engine_.list_roots();
		for (S const& root : roots_) {
			ActionDetails actions{};
			for (A const action : rules_engine.list_actions(root)) {
				// initialize all root actions as unexpanded
				actions[action] = {};
			}
			data_[root] = actions;
		}
	}

	[[nodiscard]] auto is_terminal_at(S state) const -> bool { return data_.at(state).empty(); }
	[[nodiscard]] auto is_expanded_at(S state, A action) const -> bool { return edge_count_at(state, action) > 0; }
	[[nodiscard]] auto roots() const -> std::vector<S> { return roots_; }
	[[nodiscard]] auto actions_at(S state) const -> std::vector<A> {
		std::vector<A> result;
		ActionDetails const& action_details = data_.at(state);
		result.reserve(action_details.size());
		for (auto const& entry : action_details) {
			result.push_back(entry.first);
		}
		return result;
	}

	[[nodiscard]] auto edges_at(S state, A action) const -> std::vector<ActionEdge<S>> {
		return data_.at(state).at(action);
	}

	[[nodiscard]] auto action_count_at(S state) const -> size_t { return data_.at(state).size(); }

	[[nodiscard]] auto edge_count_at(S state, A action) const -> size_t { return data_.at(state).at(action).size(); }

	auto expand_at(
		S state, A action, std::vector<ActionEdge<S>> new_edges, std::vector<std::pair<S, std::vector<A>>> next_states)
		-> bool;

	auto rules_engine() const -> R const& { return rules_engine_; }

 private:
	R rules_engine_;
	std::vector<S> roots_;
	using ActionDetails = std::unordered_map<A, std::vector<ActionEdge<S>>>;
	std::unordered_map<S, ActionDetails> data_;
};

template <typename S, typename A, RulesEngine<S, A> R>
auto DefaultGraphContainer_v1<S, A, R>::expand_at(
	S state, A action, std::vector<ActionEdge<S>> new_edges, std::vector<std::pair<S, std::vector<A>>> next_states)
	-> bool {
	if (is_expanded_at(state, action))
		return false;

	data_.at(state).at(action) = new_edges;
	for (const auto& [child, actions] : next_states) {
		if (data_.try_emplace(child).second) {
			for (A child_action : actions)
				data_.at(child).emplace(child_action, std::vector<ActionEdge<S>>());
		}
		// else do nothing: state already known/initialised (possibly visited via a different parent, etc.)
	}
	return true;
}

}  // namespace sag
