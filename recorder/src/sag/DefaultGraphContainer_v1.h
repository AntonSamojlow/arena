#pragma once
#include <algorithm>
#include <cassert>
#include <map>
#include <memory>
#include <random>
#include <unordered_map>

#include "GraphConcepts.h"

namespace sag {

/// Basic implentation of a state action graph container
template <typename S, typename A>
class DefaultGraphContainer_v1 {
 public:
	using RootData = std::vector<std::pair<S, std::vector<A>>>;

	template <RulesEngine<S, A> R>
	explicit DefaultGraphContainer_v1(R const& rules_engine) {
		reset_roots_to(rules_engine);
	}

	friend auto operator==(const DefaultGraphContainer_v1&, const DefaultGraphContainer_v1&) -> bool = default;

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

	auto add(S state, std::vector<A> actions) -> bool;

	auto clear_and_reroot(std::vector<S> new_roots) -> void {
		std::vector<std::pair<S, std::vector<A>>> root_data;
		for (S root : new_roots) {
			std::vector<A> actions;
			for (auto const [action, _] : data_[root]) {
				actions.emplace_back(action);
			}
			root_data.emplace_back(root, actions);
		}

		data_.clear();
		actions_ = 0;
		edges_ = 0;

		reset_roots_to(root_data);
	}

	auto expand_at(
		S state, A action, std::vector<ActionEdge<S>> new_edges, std::vector<std::pair<S, std::vector<A>>> next_states)
		-> bool;

	[[nodiscard]] auto action_count() const -> size_t { return actions_; }
	[[nodiscard]] auto state_count() const -> size_t { return data_.size(); }
	[[nodiscard]] auto edge_count() const -> size_t { return edges_; }

 private:
	std::vector<S> roots_;
	// listing the actions of a state requires an ordered container
	using ActionDetails = std::map<A, std::vector<ActionEdge<S>>>;
	std::unordered_map<S, ActionDetails> data_;

	size_t actions_{0};
	size_t edges_{0};

	template <RulesEngine<S, A> R>
	auto reset_roots_to(R const& rules) -> void {
		std::vector<std::pair<S, std::vector<A>>> root_data;
		for (S const root : rules.list_roots()) {
			root_data.emplace_back(root, rules.list_actions(root));
		}
		reset_roots_to(root_data);
	}

	auto reset_roots_to(std::vector<std::pair<S, std::vector<A>>> const& root_data) -> void {
		roots_.clear();
		roots_.reserve(root_data.size());
		for (auto [root, actions] : root_data) {
			roots_.push_back(root);
			ActionDetails action_details{};
			for (A const action : actions) {
				// initialize roots as unexpanded
				action_details[action] = {};
			}
			data_[root] = action_details;
		}
	}
};

template <typename S, typename A>
auto DefaultGraphContainer_v1<S, A>::add(S state, std::vector<A> actions) -> bool {
	if (data_.try_emplace(state).second) {
		for (A child_action : actions)
			data_.at(state).emplace(child_action, std::vector<ActionEdge<S>>());
		actions_ += actions.size();
		return true;
	}
	// else return false: state already known/initialised (possibly visited via a different parent, etc.)
	assert(data_.at(state).size() == actions.size());
	return false;
}

template <typename S, typename A>
auto DefaultGraphContainer_v1<S, A>::expand_at(
	S state, A action, std::vector<ActionEdge<S>> new_edges, std::vector<std::pair<S, std::vector<A>>> next_states)
	-> bool {
	if (is_expanded_at(state, action))
		return false;

	data_.at(state).at(action) = new_edges;
	edges_ += new_edges.size();

	for (const auto& [child, actions] : next_states) {
		add(child, actions);
	}
	return true;
}

}  // namespace sag
