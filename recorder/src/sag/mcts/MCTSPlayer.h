#include <algorithm>
#include <cassert>
#include <numeric>

#include "MCTS.h"
#include "sag/match/Player.h"
#include "sag/mcts/MCTS.h"
#include "sag/mcts/StatsContainer.h"

namespace sag::mcts {

template <Graph G>
class MCTSPlayer {
 public:
	MCTSPlayer() = default;
	explicit MCTSPlayer(size_t simulations) : rng_(std::mt19937(std::random_device()())), simulations_(simulations) {}

	MCTSPlayer(std::string_view id, size_t simulations, std::string_view name, BaseMCTS<G>&& mcts)
			: rng_(std::mt19937(std::random_device()())),
				simulations_(simulations),
				id_(id),
				name_(name),
				mcts_(std::move(mcts)) {}

	[[nodiscard]] auto id() const -> std::string { return id_; }
	[[nodiscard]] auto display_name() const -> std::string { return name_; }

	auto operator==(const MCTSPlayer& other) const -> bool = default;

	[[nodiscard]] auto choose_play(typename G::state state, typename G::container& graph, typename G::rules const& rules)
		-> typename G::action {
		for (size_t i = 0; i < simulations_; i++)
			mcts_.descend(state, stats_, graph, rules);
		std::vector<typename G::action> const actions = graph.actions_at(state);

		// transform action estimates to probabilities (values in range [0,1])
		std::vector<float> win_probabilities{};
		win_probabilities.reserve(actions.size());
		auto temp = action_estimates_at<G>(state, graph, stats_);
		std::ranges::transform(temp, std::back_inserter(win_probabilities), [](float value) { return (1 + value) / 2; });
		assert(win_probabilities.size() == actions.size());

		// pick at random acc. to probabilities
		float threshold =
			unit_distribution_(rng_) * std::accumulate(win_probabilities.begin(), win_probabilities.end(), 0.0F);
		float check_value = 0.0F;
		for (size_t i = 0; i < actions.size(); i++) {
			check_value += win_probabilities[i];
			if (check_value > threshold)
				return actions[i];
		}
		assert(false);  // should never reach this point, previous loop is expected to finish
		return actions.back();
	}

 private:
	std::mt19937 rng_;
	std::uniform_real_distribution<float> unit_distribution_ = std::uniform_real_distribution<float>(0.0, 1.0);
	std::string id_ = "default(mcts)";
	std::string name_ = "mcts-player";

	BaseMCTS<G> mcts_;
	Statistics<typename G::state> stats_ = {};
	size_t simulations_ = 1;
};

static_assert(sag::match::Player<MCTSPlayer<sag::example::Graph>, sag::example::Graph>);

}  // namespace sag::mcts
