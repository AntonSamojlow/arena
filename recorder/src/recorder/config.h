#pragma once

#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace nl = nlohmann;

namespace config {

struct MCTS {
	int simulation_count = 0;
	double explore_constant = 0.0;
	bool sample_uniformly = false;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MCTS, simulation_count, explore_constant, sample_uniformly)

struct Player {
	std::string name;
	MCTS mcts;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Player, name, mcts)

struct Recorder {
	std::string db_file_path;
	std::vector<Player> players;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Recorder, db_file_path, players)

template <class Config>  // todo: error handling
auto read(std::filesystem::path const& input_file_path) -> Config {
	std::ifstream file{input_file_path.c_str()};
	nl::json data = nl::json::parse(file);
	return data;
}

template <class Config>  // todo: error handling
auto write(Config const& config, std::filesystem::path const& output_file_path) {
	std::ofstream file{output_file_path.c_str()};
	nl::json output = config;
	file << output;
}

}  // namespace config