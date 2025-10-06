#pragma once
#include <cstddef>
#include "forward.hxx"

template<bitspan_word W> requires (!std::is_const_v<W>)
struct bit_ref final {
private:
    friend bitspan<W>;
    W*   ptr;
    char min_idx;

    constexpr bit_ref(bitspan<W> span, size_t i) noexcept
        : ptr(&span.words()[bitspan<W>::maj_bi(i)]), min_idx(bitspan<W>::min_bi(i)) {}

public:
    bit_ref() = delete;
    [[nodiscard]] operator bool() const noexcept { return ((*ptr) >> min_idx) & 1; }
    bit_ref  operator|=(bool val) const noexcept { *ptr |= W(val) << min_idx; return *this; }
    bit_ref  operator&=(bool val) const noexcept { *ptr &= W(val) << min_idx; return *this; }
    bit_ref  operator^=(bool val) const noexcept { *ptr ^= W(val) << min_idx; return *this; }
    bit_ref  operator =(bool val) const noexcept { // NOLINT, proxy reference
        *ptr = ((*ptr) & ~(W(1) << min_idx)) | (W(val) << min_idx);
        return *this;
    }
};
