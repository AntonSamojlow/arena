#pragma once

#include <fmt/format.h>

#include <exception>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <tl/expected.hpp>
#include <vector>

#include "tools/Failure.h"

namespace nl = nlohmann;

namespace app::config {

#pragma GCC diagnostic push
// reason: https://github.com/llvm/llvm-project/issues/43670
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"

// NOLINTBEGIN (*-float-equal, *-use-nullptr)

struct MCTS {
	size_t simulations = 0;
	double explore_constant = 0.0;
	bool sample_uniformly = false;
	friend auto operator<=>(const MCTS&, const MCTS&) = default;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MCTS, simulations, explore_constant, sample_uniformly)

struct Player {
	std::string name;
	MCTS mcts;
	friend auto operator<=>(const Player&, const Player&) = default;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Player, name, mcts)

struct Recorder {
	std::string db_file_path;
	std::vector<Player> players;
	size_t parallel_games = 1;

	friend auto operator<=>(const Recorder&, const Recorder&) = default;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Recorder, db_file_path, players, parallel_games)

// NOLINTEND
#pragma GCC diagnostic pop

template <class Config>
auto read(std::filesystem::path const& input_file_path) -> tl::expected<Config, tools::Failure> {
	try {
		std::ifstream file{input_file_path.c_str()};
		nl::json data = nl::json::parse(file);
		return data;
	} catch (std::exception const& exc) {
		return tl::unexpected<tools::Failure>({-1, fmt::format("Failed to parse from file: {}", exc.what())});
	}
}

template <class Config>
auto write(Config const& config, std::filesystem::path const& output_file_path) -> tl::expected<void, tools::Failure> {
	try {
		std::ofstream file{output_file_path.c_str()};
		nl::json output = config;
		file << output;
		return {};
	} catch (std::exception const& exc) {
		return tl::unexpected<tools::Failure>({-1, fmt::format("Failed to write to file: {}", exc.what())});
	}
}

}  // namespace app::config
