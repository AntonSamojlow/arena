#pragma once
#include <concepts>

#include "GraphConcepts.h"

namespace sag::mcts {

// todo: improve naming
struct StatsEntry {
	int N = 0;
	float Q = 0.0;
};

// disable until clang-format 16 with 'RequiresExpressionIndentation : OuterScope' is available
// clang-format off

template <typename C, typename Key>
concept StatsContainer = sag::Identifier<Key> && std::semiregular<C> &&
requires(const C const_container, C container, Key key, sag::Score score) {
	{ const_container.at(key) } -> std::same_as<StatsEntry>;
	{ const_container.has(key) } -> std::same_as<bool>;

	{ container.clear() } -> std::same_as<void>;
	{ container.initialize(key, score) } -> std::same_as<void>;
	{ container.add_visit(key) } -> std::same_as<void>;
	{ container.add_visit_result(key, score) } -> std::same_as<void>;
};

}  // namespace sag::mcts
