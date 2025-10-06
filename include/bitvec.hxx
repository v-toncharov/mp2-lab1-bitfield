#pragma once
#include <cstdlib>
#include <cstring>
#include <vector>
#include "bitspan.hxx"
#include "forward.hxx"

template<bitspan_word W> requires (!std::is_const_v<W>)
struct bitvec final {
public:
    // --- type associations ---
    using word        = W;
    using word_vector = std::vector<W>;
    using const_span  = bitspan<W const>;
    using mut_span    = bitspan<W>;
    template<bool mut> using words_t    = bitvec_words<mut, W>;
    template<size_t N> using word_array = std::array<W, N>;
    template<bool in>  using iter_t     = bitspan_iter<in, W>;
    template<size_t Ext = std::dynamic_extent> using word_span = std::span<W, Ext>;

    friend words_t<false>;
    friend words_t<true>;
    // --- end type associations ---

    // --- constants ---
    static constexpr size_t bits_per_word = const_span::bits_per_word;
    static constexpr size_t majshift      = const_span::majshift;
    static constexpr size_t minmask       = const_span::minmask;
    static constexpr size_t min_cap       = const_span::bits_in_words(128 / sizeof(W));
    // --- end constants ---

    // --- useful expressions ---
    [[nodiscard]] static constexpr size_t bytes_for_bitcount_unchecked(size_t c) noexcept
        { return const_span::words_for_bitcount_unchecked(c) * sizeof(W); }
    [[nodiscard]] static constexpr size_t bytes_for_bitcount(size_t c) {
        auto words = const_span::words_for_bitcount(c);
        return throwing_mul<size_t, std::bad_array_new_length>(words, sizeof(W));
    }
    // --- end useful expressions ---

private:
    // --- fields ---
    W*     _base = nullptr;
    size_t _len  = 0;
    size_t _cap  = 0;
    // --- end fields ---

    // --- memory management helpers ---
    void deallocate() {
        if (_cap == 0) return;
        _cap = 0;
        free(_base); // NOLINT
        _base = nullptr;
    }
    void reallocate(size_t bits) {
        if (bits == 0) return deallocate();
        auto words = const_span::words_for_bitcount(bits);
        auto bytes = throwing_mul<size_t, std::bad_array_new_length>(words, sizeof(W));
        auto new_ptr = (_cap != 0) ? realloc(_base, bytes) : malloc(bytes); // NOLINT
        if (new_ptr == nullptr) throw std::bad_alloc();
        _base = static_cast<W*>(new_ptr);
        _cap = const_span::bits_in_words(words);
    }
    // --- end memory management helpers ---

public:
    // --- constructors and rule of five ---
    bitvec() noexcept = default;
    bitvec& operator=(bitvec const& o) {
        if (this == &o) return *this;
        reserve_for_exact(o._len);
        memcpy(_base, o._base, o.words().count() * sizeof(W));
        _len = o._len;
        return *this;
    }
    bitvec(bitvec const& o) : bitvec() { *this = o; }
    explicit bitvec(size_t len) : bitvec() { resize(len); }

    bitvec(bitvec&& o) noexcept : _base(o._base), _len(o._len), _cap(o._cap)
        { o._base = nullptr; o._len = o._cap = 0; }
    bitvec& operator=(bitvec&& o) noexcept
        { if (this != &o) { this->~bitvec(); new(this) bitvec(std::move(o)); } return *this; }

    ~bitvec() { if (_cap != 0) free(_base); } // NOLINT
    // --- end constructors and rule of five ---

    // --- memory management ---
    size_t reserve_for_exact(size_t new_cap) {
        if (_cap < new_cap) [[unlikely]] reallocate(new_cap);
        return _cap;
    }
    size_t reserve_for(size_t new_cap) {
        auto amort_siz = (_len << 1 >= _len) ? _len << 1 : SIZE_MAX;
        return reserve_for_exact(std::max(amort_siz, new_cap));
    }
    size_t resize(size_t new_len) {
        reserve_for(new_len);
        if (new_len > _len) {
            auto new_wordcnt = const_span::words_for_bitcount_unchecked(new_len);
            memset(words().end(), 0, (new_wordcnt - words().count()) * sizeof(W));
        }
        _len = new_len;
        if (new_len != 0) *(words().end() - 1) &= (1 << const_span::min_bi(new_len)) - 1;
        return new_len;
    }
    // --- end memory management ---

    // --- accessors ---
    [[nodiscard]] size_t len() const noexcept { return _len; }
    [[nodiscard]] size_t cap() const noexcept { return _cap; }
    size_t truncate(size_t len) noexcept { return _len = std::min(len, _len); }
    // --- end accessors ---

    // --- span acquisition ---
    [[nodiscard]] operator bitspan<W const>() const & noexcept
        { return bitspan<W const>(_base, _len); }
    [[nodiscard]] operator bitspan<W      >()       & noexcept
        { return bitspan<W      >(_base, _len); }
    [[nodiscard]] operator bitspan<W const>() && = delete;
    [[nodiscard]] operator bitspan<W      >() && = delete;

    [[nodiscard]] bitspan<W const> span() const & noexcept
        { return bitspan<W const>(*this); }
    [[nodiscard]] bitspan<W      > span()       & noexcept
        { return bitspan<W      >(*this); }
    [[nodiscard]] bitspan<W> span() && = delete;
    // --- end span acquisition ---

    // --- misc utilities ---
    template<bitspan_word O>
    void ensure_eq_length(bitspan<O> o) const
        { if (_len != o.len()) throw bitspan_length_mismatch(); }
    template<bitspan_word O>
    void ensure_eq_length(bitvec<O> const& o) const { ensure_eq_length(o.span()); }
    // --- end misc utilities ---

    /// --- indexing ---
    [[nodiscard]] bool       operator[](size_t i) const { return span()[i]; }
    [[nodiscard]] bit_ref<W> operator[](size_t i)       { return span()[i]; }
    /// --- end indexing ---

    /// --- bulk bitwise operations ---
    [[nodiscard]] bool operator ==(bitspan<W const> o) const noexcept { return span()==o; }
    [[nodiscard]] bool operator ==(bitvec const&    o) const noexcept { return span()==o.span(); }
    bitvec& reset      (bool val = false  ) { span().reset(val);  return *this; }
    bitvec& invert     (                  ) { span().invert();    return *this; }
    bitvec& operator &=(bitspan<W const> o) { span() &= o;        return *this; }
    bitvec& operator |=(bitspan<W const> o) { span() |= o;        return *this; }
    bitvec& operator ^=(bitspan<W const> o) { span() ^= o;        return *this; }
    bitvec& set_from   (bitspan<W const> o) { span().set_from(o); return *this; }

    bitvec& operator &=(bitvec    const& o) { return *this &= o.span();  }
    bitvec& operator |=(bitvec    const& o) { return *this |= o.span();  }
    bitvec& operator ^=(bitvec    const& o) { return *this ^= o.span();  }
    bitvec& set_from   (bitvec    const& o) { return set_from(o.span()); }

    [[nodiscard]] bitvec operator ~() const
        { auto rslt = *this; rslt.invert(); return rslt; }
    [[nodiscard]] bitvec operator &(bitspan<W const> o) const
        { auto rslt = *this; rslt.resize(std::max(rslt.len(), o.len())); rslt &= o; return rslt; }
    [[nodiscard]] bitvec operator |(bitspan<W const> o) const
        { auto rslt = *this; rslt.resize(std::max(rslt.len(), o.len())); rslt |= o; return rslt; }
    [[nodiscard]] bitvec operator ^(bitspan<W const> o) const
        { auto rslt = *this; rslt.resize(std::max(rslt.len(), o.len())); rslt ^= o; return rslt; }
    [[nodiscard]] bitvec operator &(bitvec const& o) const { return *this & o.span(); }
    [[nodiscard]] bitvec operator |(bitvec const& o) const { return *this | o.span(); }
    [[nodiscard]] bitvec operator ^(bitvec const& o) const { return *this ^ o.span(); }
    /// --- end bulk bitwise operations ---

    /// --- helper constructors ---
    [[nodiscard]] words_t<false> words() const noexcept { return {*this}; }
    [[nodiscard]] words_t<true > words()       noexcept { return {*this}; }
    [[nodiscard]] indices bit_indices () const noexcept { return indices(_len); }
    [[nodiscard]] indices word_indices() const noexcept { return indices(words().count()); }
    template<bool in> [[nodiscard]] iter_t<in> iter() const noexcept
        { return span().iter(); }

    /// --- end helper constructors ---
};

template<bool mut, bitspan_word W> requires (!std::is_const_v<W>)
struct bitvec_words final {
private:
    friend bitvec<W>;
    friend bitvec_words<true, W>;
    std::conditional_t<mut, bitvec<W>&, bitvec<W> const&> vec;

    bitvec_words(decltype(vec) vec) noexcept : vec(vec) {}

public:
    using deref_type = std::conditional_t<mut, W, const W>;

    bitvec_words(bitvec_words<false, W> o) noexcept : vec(o.vec) {}
    [[nodiscard]] deref_type* begin() const noexcept { return vec._base; }
    [[nodiscard]] deref_type* end  () const noexcept { return begin() + count(); }
    [[nodiscard]] W const*   cbegin() const noexcept { return begin(); }
    [[nodiscard]] W const*   cend  () const noexcept { return end(); }
    [[nodiscard]] size_t      count() const noexcept
        { return bitspan<W>::words_for_bitcount_unchecked(vec._len); }
    [[nodiscard]] deref_type& operator[](size_t i) const noexcept { return begin()[i]; }
    [[nodiscard]] deref_type& of_bit    (size_t i) const noexcept
        { return begin()[bitspan<W>::maj_bi(i)]; }
};

template<bitspan_word W>
std::ostream& operator<<(std::ostream& o, bitvec<W> const& b) { return o << b.span(); }

/// --- explicit instantiation ---
template struct bitvec<>;
/// --- end explicit instantiation ---
