#pragma once
#include <concepts>
#include <cstdint>
#include <limits>

using default_bitspan_word = uintptr_t;

template<typename W>
concept bitspan_word = std::unsigned_integral<W> && requires(W w) {
    requires std::numeric_limits<W>::radix  == 2;
    requires std::numeric_limits<W>::digits <= std::numeric_limits<char>::max();
};
