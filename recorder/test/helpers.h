#pragma once
#include <filesystem>
#include <random>
#include <string>

#include "tools/UniqueResource.h"

namespace test {
struct RNG {
	static auto instance() -> RNG & {
		static RNG instance;
		return instance;
	}  // instance

	auto unit_value() -> double { return unit_interval_(rng_); }
	auto nonnegative_number() -> int { return positve_intergers_(rng_); }

	// explicitely delete
	RNG(RNG &&) = delete;
	auto operator=(RNG &&) -> RNG & = delete;
	RNG(const RNG &) = delete;
	auto operator=(const RNG &) -> RNG & = delete;

 private:
	std::mt19937 rng_ = std::mt19937(std::random_device()());
	std::uniform_real_distribution<double> unit_interval_ = std::uniform_real_distribution<double>(0.0, 1.0);
	std::uniform_int_distribution<int> positve_intergers_ = std::uniform_int_distribution<int>(0);

	RNG() = default;
	~RNG() = default;
};

struct FileDeleter {
	auto operator()(std::string const &file_path) { std::filesystem::remove(file_path); }
};
using TempFilePath = tools::unique_resource<std::string, FileDeleter>;

/// Returns a randomly generated TempFilePath (handle for a self-removing file path).
/// Throws if the randomly chosen path exists.
auto unique_file_path(bool create_file) -> TempFilePath;

}  // namespace test
