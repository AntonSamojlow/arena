#pragma once
#include <random>

#include "GraphConcepts.h"
#include "MCTSConcepts.h"

namespace sag::mcts {

template <sag::Identifier S>
struct Path : std::vector<S> {
	bool terminal = false;
};

template <sag::Identifier S>
auto update(Path<S> const & path, StatsContainer<S> auto& container) -> void {
	if (path.empty())
		return;

	float end_value = container.at(path.back()).Q;
	int sign = 2 * (path.size() % 2) - 1;
	for (auto it = path.begin(); it < path.end() - 1; ++it, sign *= -1)
		container.add_visit_result(*it, Score(static_cast<float>(sign) * end_value));

	container.add_visit(path.back());  // to increment N
}

//
//template <typename State, typename Action>
//	requires sag::Vertices<State, Action>
//class MCTS {
// public:
//	struct Config {
//		double explore_constant = 0.5;
//		bool sample_actions_uniformly = true;
//	};
//
// private:
//	std::mt19937 rng_;
//	std::uniform_real_distribution<double> unit_distribution_;
//}
//
//template <typename State, typename Action>
//	requires sag::Vertices<State, Action>
//auto descend(State state, sag::) -> void {}

}  // namespace sag::mcts
