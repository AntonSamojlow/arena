#pragma once

#include <fmt/format.h>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>

#include <exception>
#include <fstream>
#include <nlohmann/json.hpp>
#include <optional>
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

struct SimpleLog {
	spdlog::level::level_enum level = spdlog::level::info;
	std::string pattern = "[%H:%M:%S.%e] %v";

	friend auto operator<=>(const SimpleLog&, const SimpleLog&) = default;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SimpleLog, level, pattern)

struct FileLog : SimpleLog {
	std::string folder;
	size_t max_size_mb = 1;
	size_t max_files = 5;
	bool rotate_on_open = false;
	friend auto operator<=>(const FileLog&, const FileLog&) = default;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FileLog, level, pattern, folder, max_size_mb, max_files, rotate_on_open)

struct Log {
	std::optional<SimpleLog> console;
	std::optional<FileLog> file;
	friend auto operator<=>(const Log&, const Log&) = default;
};

template <class T>
void optional_to_json(nl::json& j, const char* name, const std::optional<T>& value) {
	if (value)
		j[name] = *value;
}

template <class T>
void optional_from_json(const nl::json& j, const char* name, std::optional<T>& value) {
	const auto it = j.find(name);
	if (it != j.end())
		value = it->get<T>();
	else
		value = std::nullopt;
}

static auto to_json(nl::json& json, const Log& log) -> void {
	optional_to_json(json, "console", log.console);
	optional_to_json(json, "file", log.file);
}

static void from_json(const nl::json& json, Log& log) {
	optional_from_json(json, "console", log.console);
	optional_from_json(json, "file", log.file);
}

struct Recorder {
	std::string db_file_path;
	size_t parallel_games = 1;
	std::vector<Player> players;
	Log log;

	friend auto operator<=>(const Recorder&, const Recorder&) = default;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Recorder, db_file_path, parallel_games, players, log)
// NOLINTEND

template <class Config>
auto read(std::filesystem::path const& input_file_path) -> tl::expected<Config, tools::Failure> {
	try {
		std::ifstream file{input_file_path.c_str()};
		nl::json data = nl::json::parse(file);
		return data;
	} catch (std::exception const& exc) {
		return tl::unexpected<tools::Failure>(tools::Failure{-1, fmt::format("Failed to parse from file: {}", exc.what())});
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
		return tl::unexpected<tools::Failure>(tools::Failure{-1, fmt::format("Failed to write to file: {}", exc.what())});
	}
}

}  // namespace app::config
