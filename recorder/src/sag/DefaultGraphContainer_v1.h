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
		set_roots(rules_engine);
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

	auto clear_non_roots() -> void {
		std::unordered_map<S, ActionDetails> only_roots;
		for (S const root : roots_) {
			initialize_root_in(only_roots, root, data_[root]);
		}
		data_.swap(only_roots);
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

	auto initialize_root_in(std::unordered_map<S, ActionDetails>& target, S root, std::vector<A> const& actions) -> void {
		ActionDetails action_details{};
		for (A const action : actions) {
			// initialize roots as unexpanded
			action_details[action] = {};
		}
		target[root] = action_details;
	}

	template <RulesEngine<S, A> R>
	auto set_roots(R const& rules) -> void {
		roots_.clear();
		auto root_list = rules.list_roots();
		roots_.reserve(root_list.size());
		for (S const root : root_list) {
			roots_.push_back(root);
			initialize_root_in(data_, root, rules.list_actions(root));
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
