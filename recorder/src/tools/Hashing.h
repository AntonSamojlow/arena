#include <functional>

namespace tools {

// "standard" implementation from boost
template <class T>
inline void hash_combine(std::size_t& seed, const T& value) {
	std::hash<T> hasher;
	// NOLINTNEXTLINE(*magic-numbers,*-signed-bitwise)
	seed ^= hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

}  // namespace tools

template <class T, size_t N>
// NOLINTNEXTLINE(cert-dcl58-cpp)
struct std::hash<std::array<T, N>> {
	auto operator()(std::array<T, N> const& array) const noexcept -> std::size_t {
		size_t result = 0;
		for (size_t i = 0; i < array.size(); ++i) {
			tools::hash_combine(result, array[i]);
		}
		return result;
	}
};
