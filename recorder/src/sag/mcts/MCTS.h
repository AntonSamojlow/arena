#include <random>

#include "GraphConcepts.h"
#include "MCTSConcepts.h"

namespace sag::mcts {

template <sag::Identifier S>
struct Path : std::vector<S> {
	bool terminal = false;
};

template <typename State, typename Action>
	requires sag::Vertices<State, Action>
class MCTS {
 public:
	struct Config {
		double explore_constant = 0.5;
		bool sample_actions_uniformly = true;
	};
	--WORK_IN_PROGRESS--
 private:
	std::mt19937 rng_;
	std::uniform_real_distribution<double> unit_distribution_;
}

template <typename State, typename Action>
	requires sag::Vertices<State, Action>
auto descend(State state, sag::) -> void {}

}  // namespace sag::mcts