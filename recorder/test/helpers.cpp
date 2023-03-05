#include "helpers.h"

#include <stdio.h>

#include <filesystem>
#include <stdexcept>

namespace test {

auto unique_file_path(bool create_file) -> TempFilePath {
	std::string new_file = "test_tempfile_" + std::to_string((RNG::instance().nonnegative_number()));

	if (std::filesystem::exists(new_file))
		throw std::runtime_error("path '" + new_file + "' already exists");

	TempFilePath tempfile;
	tempfile.reset(new_file);

	if (create_file) {
		FILE* file_handle;
		if (0 != ::fopen_s(&file_handle, new_file.c_str(), "w+")) {
			throw std::runtime_error("failed to open (create) file: " + new_file);
		}
		if (0 != std::fclose(file_handle)) {
			throw std::runtime_error("failed to close file: " + new_file);
		};
	}

	return tempfile;
}

}  // namespace test
