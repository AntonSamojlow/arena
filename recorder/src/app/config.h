#pragma once

#include <fmt/format.h>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>

#include <exception>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <tl/expected.hpp>
#include <vector>

#include "tools/Failure.h"
namespace nl = nlohmann;

namespace spdlog::level {

// NOLINTBEGIN: nlohmann macros violate plenty checks
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"  // silence gcc not knowing clang-specific pragmas
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
NLOHMANN_JSON_SERIALIZE_ENUM(level_enum,
	{
		{off, "off"},
		{critical, "critical"},
		{trace, "trace"},
		{info, "info"},
		{err, "err"},
		{warn, "warn"},
		{debug, "debug"},
	})
#pragma GCC diagnostic pop
// NOLINTEND

}  // namespace spdlog::level

namespace app::config {

// NOLINTBEGIN: nlohmann macros violate plenty checks
struct MCTS {
	float explore_constant = 0.0F;
	bool sample_uniformly = false;
	size_t simulations = 0;
	friend auto operator<=>(const MCTS&, const MCTS&) = default;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MCTS, explore_constant, sample_uniformly, simulations)

struct Player {
	std::string name;
	MCTS mcts;
	friend auto operator<=>(const Player&, const Player&) = default;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Player, name, mcts)

struct Recorder {
	std::string db_file_path;
	size_t parallel_games = 1;
	std::vector<Player> players;
	spdlog::level::level_enum log_level = spdlog::level::info;

	friend auto operator<=>(const Recorder&, const Recorder&) = default;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Recorder, db_file_path, parallel_games, players, log_level)
// NOLINTEND

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
