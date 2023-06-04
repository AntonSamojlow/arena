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