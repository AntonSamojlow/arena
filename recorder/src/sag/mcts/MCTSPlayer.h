#include <algorithm>
#include <cassert>
#include <cmath>
#include <concepts>
#include <iterator>
#include <numeric>
#include <optional>

#include "MCTS.h"
#include "sag/match/Player.h"
#include "sag/mcts/MCTS.h"
#include "sag/mcts/StatsContainer.h"
#include "tools/BoundedValue.h"

namespace sag::mcts {

template <Graph G>
class MCTSPlayer : public match::ProbabilisticPlayer<G> {
 public:
	MCTSPlayer() : match::ProbabilisticPlayer<G>("default(mcts)", "mcts-player") {}
	explicit MCTSPlayer(size_t simulations)
			: match::ProbabilisticPlayer<G>("default(mcts)", "mcts-player"), simulations_(simulations) {}

	MCTSPlayer(size_t simulations, std::optional<tools::NonNegative> estimates_exponent)
			: match::ProbabilisticPlayer<G>("default(mcts)", "mcts-player"),
				simulations_(simulations),
				estimates_exponent_(estimates_exponent) {}

	MCTSPlayer(std::string_view id,
		size_t simulations,
		std::optional<tools::NonNegative> estimates_exponent,
		std::string_view name,
		BaseMCTS<G>&& mcts)
			: match::ProbabilisticPlayer<G>(id, name),
				mcts_(std::move(mcts)),
				simulations_(simulations),
				estimates_exponent_(estimates_exponent) {}

	auto operator==(const MCTSPlayer& other) const -> bool = default;

	[[nodiscard]] auto choose_play(typename G::state state, typename G::container& graph, typename G::rules const& rules)
		-> typename G::action override {
		for (size_t i = 0; i < simulations_; i++)
			mcts_.descend(state, stats_, graph, rules);

		std::vector<typename G::action> const actions = graph.actions_at(state);
		auto estimates_raw = action_estimates_at<G>(state, graph, stats_);
		assert(estimates_raw.size() == actions.size());

		// free memory of mcts stats
		stats_.clear();

		if (!estimates_exponent_.has_value()) {
			auto min_index = static_cast<size_t>(
				std::distance(estimates_raw.begin(), std::min_element(estimates_raw.begin(), estimates_raw.end())));
			return actions[min_index];
		}

		// Transform raw action estimates into a probability distribution,
		// describing the win chance of the active player:
		std::vector<float> estimates_distribution{};
		estimates_distribution.reserve(actions.size());

		// First invert, shift values into range [0, 1] each and apply estimates_exponent
		std::ranges::transform(estimates_raw, std::back_inserter(estimates_distribution), [this](float value) {
			return std::pow((1 - value) / 2, estimates_exponent_->value());
		});
		// Then, normalize
		float const total_weight = std::accumulate(estimates_distribution.begin(), estimates_distribution.end(), 0.0F);
		float const uniform_value = 1.0F / static_cast<float>(estimates_distribution.size());
		std::ranges::transform(estimates_distribution,
			estimates_distribution.begin(),
			[&total_weight, &uniform_value](float value) { return total_weight > 0 ? value / total_weight : uniform_value; });

		// Pick action at random acc. to distribution
		float threshold = match::ProbabilisticPlayer<G>::roll().value();
		float check_value = 0.0F;
		for (size_t i = 0; i < actions.size(); i++) {
			check_value += estimates_distribution[i];
			if (check_value > threshold)
				return actions[i];
		}

		assert(false);  // should never reach this point, previous loop is expected to finish
		return actions.back();
	}

 private:
	BaseMCTS<G> mcts_;
	Statistics<typename G::state> stats_ = {};
	size_t simulations_ = 1;

	/// The exponent applied to action estimates when choosing the play. Nullopt (the default) means taking the best (no
	/// probabilistic coice).
	std::optional<tools::NonNegative> estimates_exponent_ = std::nullopt;
};

static_assert(std::regular<MCTSPlayer<sag::example::Graph>>);

}  // namespace sag::mcts
