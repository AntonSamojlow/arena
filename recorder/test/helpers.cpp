#include "helpers.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace test {

auto unique_file_path(bool create_file) -> TempFilePath {
	std::string new_file = "test_tempfile_" + std::to_string((RNG::instance().nonnegative_number()));
	std::filesystem::path path(new_file);
	if (std::filesystem::exists(path))
		throw std::runtime_error("path '" + path.string() + "' already exists");

	TempFilePath tempfile;
	tempfile.reset(new_file);

	if (create_file) {
		std::ofstream{path};
	}

	return tempfile;
}

}  // namespace test
