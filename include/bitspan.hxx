#pragma once
#include <bit>
#include <cstring>
#include <limits>
#include <optional>
#include <ostream>
#include <span>
#include <stdexcept>
#include <type_traits>
#include "bit_ref.hxx"
#include "checked_arith.hxx"
#include "forward.hxx"
#include "idx_iter.hxx"

struct bitspan_length_mismatch final : std::logic_error
    { bitspan_length_mismatch()     : std::logic_error("bitspan lengths did not match"){}};
struct bitspan_bitcount_overflow final : std::length_error
    { bitspan_bitcount_overflow()   : std::length_error("overflow calculating bit count"){}};
struct bitspan_word_count_overflow final : std::length_error
    { bitspan_word_count_overflow() : std::length_error("overflow calculating word count"){}};

template<bitspan_word W>
struct bitspan final {
public:
    // --- type associations ---
    using word    = W;
    using bit_ref = bit_ref<std::remove_const_t<W>>;
    using words_t = bitspan_words<W>;
    template<bool in>  using iter_t     = bitspan_iter<in, std::remove_const_t<W>>;
    template<size_t N> using word_array = std::array<W, N>;
    template<size_t Ext = std::dynamic_extent> using word_span = std::span<W, Ext>;

    friend struct bitspan<const W>;
    friend struct bitspan<std::remove_const_t<W>>;
    friend words_t;
    // --- end type associations ---

    // --- constants ---
    static constexpr size_t bits_per_word = std::numeric_limits<W>::digits;
    static constexpr size_t majshift      = std::bit_width(bits_per_word) - 1;
    static constexpr size_t minmask       = bits_per_word - 1;
    // --- end constants ---

private:
    // --- fields ---
    W*     _base = nullptr;
    size_t _len  = 0;
    // --- end fields ---

public:
    // --- useful expressions ---
    [[nodiscard]] static constexpr size_t maj_bi(size_t i) noexcept { return i >> majshift; }
    [[nodiscard]] static constexpr size_t min_bi(size_t i) noexcept { return i &  minmask ; }
    [[nodiscard]] static constexpr size_t bits_in_words(size_t num_words)
        { return throwing_mul<W, bitspan_bitcount_overflow>(num_words, bits_per_word); }
    [[nodiscard]] static constexpr size_t words_for_bitcount_unchecked(size_t c) noexcept
        { return maj_bi(c + minmask); }
    [[nodiscard]] static constexpr size_t words_for_bitcount(size_t c)
        { return maj_bi(throwing_add<size_t, bitspan_word_count_overflow>(c, minmask)); }

    static constexpr void word_to_chars(char out[bits_per_word], std::remove_const_t<W> word)
    noexcept { for (size_t i = 0; i < bits_per_word; word >>= 1) out[i++] = '0' + (word & 1); }
    // --- end useful expressions ---

    // --- constructors ---
    bitspan(bitspan<std::remove_const_t<W>> o) noexcept
        : _base(o._base), _len(o._len) {}

    // From raw parts
    explicit bitspan(W* base, size_t len) noexcept : _base(base), _len(len) {}

    template<size_t E>
    bitspan(std::span<W, E> a) : _base(a.data()), _len(bits_in_words(a.size())) {}

    template<size_t N>
    bitspan(std::array<W, N>& a) : bitspan(std::span(a)) {}
    template<size_t N>
    bitspan(std::array<W, N> const& a) requires(std::is_const_v<W>)
        : bitspan(std::span(a)) {}

    // --- end constructors ---

    // --- accessors ---
    [[nodiscard]] size_t len() const noexcept { return _len; }
    size_t truncate(size_t len) noexcept { return _len = std::min(len, _len); }
    // --- end accessors ---

    // --- misc utilities ---
    [[nodiscard]] bitspan<const W> to_const() const noexcept { return *this; }
    template<bitspan_word O>
    void ensure_ge_length(bitspan<O> o) const
        { if (_len < o._len) throw bitspan_length_mismatch(); }
    [[nodiscard]] size_t residual_bitcount() const noexcept { return _len & minmask; }
    [[nodiscard]] W residual_mask() const noexcept { return (1 << residual_bitcount()) - 1; }
    // --- end misc utilities ---

    /// --- indexing ---
    [[nodiscard]] bool    operator[](size_t i) const requires( std::is_const_v<W>) {
        if (i > _len) throw std::out_of_range("bitspan index out of range");
        return (_base[maj_bi(i)] >> min_bi(i)) & 1;
    }
    [[nodiscard]] bit_ref operator[](size_t i) const requires(!std::is_const_v<W>) {
        if (i > _len) throw std::out_of_range("bitspan index out of range");
        return bit_ref(*this, i);
    }
    /// --- end indexing ---

    /// --- bulk bitwise operations ---
    bool operator ==(bitspan<W const> o) const noexcept {
        if (_len != o._len) return false;
        size_t main_loop_words = words().count();
        if (residual_bitcount() != 0) main_loop_words--;
        for (size_t i = 0; i < main_loop_words; i++) if (_base[i] != o._base[i]) return false;
        if (residual_bitcount() != 0) {
            W resmask = residual_mask();
            size_t residx = o.words().count() - 1;
            if ((_base[residx] & resmask) != (o._base[residx] & resmask)) return false;
        }
        return true;
    }

    bitspan reset(bool val = false) const noexcept requires(!std::is_const_v<W>) // NOLINT yesdiscard
        {  return _reset_word_range(0, words().count(), val); }
    bitspan _reset_word_range(size_t start, size_t end, bool val = false) // NOLINT
    const noexcept requires(!std::is_const_v<W>) {
        W wval = 0 - W(val);
        memset(static_cast<void*>(_base + start), wval, (end - start) * sizeof(W));
        return *this;
    }

    bitspan invert() const noexcept requires(!std::is_const_v<W>) { // NOLINT yesdiscard
        for (auto i : word_indices()) _base[i] = ~_base[i];
        return *this;
    }
    bitspan operator &=(bitspan<const W> o) const requires(!std::is_const_v<W>) {
        ensure_ge_length(o);
        size_t main_loop_words = words().count();
        if (o.residual_bitcount() != 0) main_loop_words--;
        for (size_t i = 0; i < main_loop_words; i++) _base[i] &= o._base[i];
        if (o.residual_bitcount() != 0) {
            W resmask = o.residual_mask();
            size_t residx = o.words().count() - 1;
            _base[residx] &= o._base[residx] & resmask;
            // Zero-extend the smaller operand
            _reset_word_range(o.words().count(), words().count());
        }
        return *this;
    }
    bitspan operator |=(bitspan<const W> o) const requires(!std::is_const_v<W>) {
        ensure_ge_length(o);
        size_t main_loop_words = words().count();
        if (o.residual_bitcount() != 0) main_loop_words--;
        for (size_t i = 0; i < main_loop_words; i++) _base[i] |= o._base[i];
        if (o.residual_bitcount() != 0) {
            W resmask = o.residual_mask();
            size_t residx = o.words().count() - 1;
            _base[residx] |= o._base[residx] & resmask;
        }
        return *this;
    }
    bitspan operator ^=(bitspan<const W> o) const requires(!std::is_const_v<W>) {
        ensure_ge_length(o);
        size_t main_loop_words = words().count();
        if (o.residual_bitcount() != 0) main_loop_words--;
        for (size_t i = 0; i < main_loop_words; i++) _base[i] ^= o._base[i];
        if (o.residual_bitcount() != 0) {
            W resmask = o.residual_mask();
            size_t residx = o.words().count() - 1;
            _base[residx] ^= o._base[residx] & resmask;
        }
        return *this;
    }
    bitspan set_from(bitspan<const W> o) const requires(!std::is_const_v<W>) { // NOLINT
        ensure_ge_length(o);
        size_t main_loop_words = words().count();
        if (o.residual_bitcount() != 0) main_loop_words--;
        for (size_t i = 0; i < main_loop_words; i++) _base[i] = o._base[i];
        if (o.residual_bitcount() != 0) {
            W resmask = o.residual_mask();
            size_t residx = o.words().count() - 1;
            _base[residx] = o._base[residx] & resmask;
            // Zero-extend the smaller operand
            _reset_word_range(o.words().count(), words().count());
        }
        return *this;
    }
    /// --- end bulk bitwise operations ---

    /// --- helper constructors ---
    [[nodiscard]] words_t words       () const noexcept { return {*this}; }
    [[nodiscard]] indices bit_indices () const noexcept { return indices(_len); }
    [[nodiscard]] indices word_indices() const noexcept { return indices(words().count()); }
    template<bool in>
    [[nodiscard]] iter_t<in> iter() const noexcept
        { return { .span = *this,
                 .idx = 0,
                 .current = 0,
                 .cur_idx = bits_per_word }; }

    /// --- end helper constructors ---
};

template<bool in, bitspan_word W> requires (!std::is_const_v<W>)
struct bitspan_iter final {
private:
    friend struct bitspan<      W>;
    friend struct bitspan<const W>;

    bitspan<const W> span;
    size_t           idx;
    W                current;
    char             cur_idx;

    bitspan_iter(bitspan<const W> span) noexcept
        : span(span), idx(0), current(0), cur_idx(bitspan<W>::bits_per_word) {}

public:
    bitspan_iter() = delete;

    std::optional<size_t> next() noexcept {
        while (current == 0) {
            idx += bitspan<W>::bits_per_word - cur_idx;
            cur_idx = bitspan<W>::bits_per_word;
            if (idx > span.len()) return std::nullopt;
            current = span.words().of_bit(idx) ^ ((in) ? 0 : ~0);
            cur_idx = 0;
        }
        // There can't not be a bit set to 1 in current by this point, so the
        // below figure cannot be greater than the remaining number of bits
        // to process in current
        char shift = std::countr_zero(current) + 1;
        current  >>= shift;
        cur_idx   += shift;
        idx       += shift;
        return std::make_optional(idx - 1);
    }
};

template<bitspan_word W>
struct bitspan_words final {
private:
    friend bitspan<W>;
    friend bitspan_words<const W>;
    friend bitspan_words<std::remove_const_t<W>>;
    bitspan<W> span;

    bitspan_words(bitspan<W> span) noexcept : span(span) {}

public:
    bitspan_words() = delete;
    bitspan_words(bitspan_words<std::remove_const_t<W>> o) noexcept : span(o.span) {}
    [[nodiscard]] W      *  begin() const noexcept { return span._base; }
    [[nodiscard]] W const* cbegin() const noexcept { return begin(); }
    [[nodiscard]] W      *  end  () const noexcept { return begin() + count(); }
    [[nodiscard]] W const* cend  () const noexcept { return end(); }
    [[nodiscard]] size_t count() const noexcept
        { return bitspan<W>::words_for_bitcount_unchecked(span._len); }
    [[nodiscard]] W& operator[](size_t i) const noexcept { return begin()[i]; }
    [[nodiscard]] W& of_bit    (size_t i) const noexcept
        { return begin()[bitspan<W>::maj_bi(i)]; }
};

template<bitspan_word W>
std::ostream& operator<<(std::ostream& o, bitspan<W> b) {
    char buf[bitspan<W>::bits_per_word];
    size_t end = b.len() >> bitspan<W>::majshift;
    for (size_t i = 0; i < end; i++) {
        if (!o) return o;
        bitspan<W>::word_to_chars(buf, b.words()[i]);
        o.write(buf, bitspan<W>::bits_per_word);
    }
    if (!o) return o;
    size_t tail_len = b.len() & bitspan<W>::minmask;
    if (tail_len != 0) {
        bitspan<W>::word_to_chars(buf, b.words()[end]);
        o.write(buf, tail_len);
    }
    return o;
}

/// --- explicit instantiation ---
template struct bitspan<>;
template struct bitspan_iter<false>;
template struct bitspan_iter<true>;
/// --- end explicit instantiation ---
