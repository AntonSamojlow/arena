// NOLINTBEGIN (*magic-numbers)

namespace tools::literals::memory {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wc++20-compat"

consteval size_t operator""_KiB(unsigned long long int x) {
	return 1024ULL * x;
}

consteval size_t operator""_MiB(unsigned long long int x) {
	return 1024_KiB * x;
}

consteval size_t operator""_GiB(unsigned long long int x) {
	return 1024_MiB * x;
}

consteval size_t operator""_TiB(unsigned long long int x) {
	return 1024_GiB * x;
}

consteval size_t operator""_PiB(unsigned long long int x) {
	return 1024_TiB * x;
}

}  // namespace tools::literals::memory

#pragma GCC diagnostic pop
// NOLINTEND (*magic-numbers)
