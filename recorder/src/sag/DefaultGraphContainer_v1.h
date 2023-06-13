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

	/// Constructs a defult graph container from the provided root_data
	explicit DefaultGraphContainer_v1(RootData const& root_data) {
		roots_.reserve(root_data.size());
		for (auto const& [root, actions] : root_data) {
			if (std::ranges::find(roots_, root) != roots_.end())
				throw std::invalid_argument("inconsistent root data - duplicate root entry");
			roots_.push_back(root);
			ActionDetails action_details{};
			for (A const action : actions) {
				// initialize all root actions as unexpanded
				action_details[action] = {};
			}
			data_[root] = action_details;
		}
	}

	template <RulesEngine<S, A> R>
	explicit DefaultGraphContainer_v1(R const& rules_engine) : DefaultGraphContainer_v1(get_root_data(rules_engine)) {}

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

	auto expand_at(
		S state, A action, std::vector<ActionEdge<S>> new_edges, std::vector<std::pair<S, std::vector<A>>> next_states)
		-> bool;

 private:
	std::vector<S> roots_;
	// listing the actions of a state requires an ordered container
	using ActionDetails = std::map<A, std::vector<ActionEdge<S>>>;
	std::unordered_map<S, ActionDetails> data_;

	template <RulesEngine<S, A> R>
	static auto get_root_data(R const& rules) -> RootData {
		RootData rootData{};
		auto roots = rules.list_roots();
		rootData.reserve(roots.size());
		for (S const root : roots) {
			rootData.push_back({root, rules.list_actions(root)});
		}
		return rootData;
	}
};

template <typename S, typename A>
auto DefaultGraphContainer_v1<S, A>::add(S state, std::vector<A> actions) -> bool {
	if (data_.try_emplace(state).second) {
		for (A child_action : actions)
			data_.at(state).emplace(child_action, std::vector<ActionEdge<S>>());
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
	for (const auto& [child, actions] : next_states) {
		add(child, actions);
	}
	return true;
}

}  // namespace sag
