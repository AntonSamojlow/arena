#include <vcruntime.h>

#include <functional>

namespace tools {

// "standard" implementation from boost
template <class T>
inline void hash_combine(std::size_t& seed, const T& value) {
	std::hash<T> hasher;
	// NOLINTNEXTLINE(*magic-numbers,*-signed-bitwise)
	seed ^= hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template <class T, size_t N>
struct std::hash<std::array<T, N>> {
	auto operator()(std::array<T, N> const& array) const noexcept -> std::size_t {
		size_t hash = 0;
		for (size_t i = 0; i < array.size(); ++i) {
			tools::hash_combine(hash, array[i]);
		}
		return hash;
	}
};

}  // namespace tools
