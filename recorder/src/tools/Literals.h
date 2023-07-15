// NOLINTBEGIN (*magic-numbers)

namespace tools::literals::memory {

constexpr size_t operator""_KiB(unsigned long long int x) {
	return 1024ULL * x;
}

constexpr size_t operator""_MiB(unsigned long long int x) {
	return 1024_KiB * x;
}

constexpr size_t operator""_GiB(unsigned long long int x) {
	return 1024_MiB * x;
}

constexpr size_t operator""_TiB(unsigned long long int x) {
	return 1024_GiB * x;
}

constexpr size_t operator""_PiB(unsigned long long int x) {
	return 1024_TiB * x;
}

}  // namespace tools::literals::memory

// NOLINTEND (*magic-numbers)
