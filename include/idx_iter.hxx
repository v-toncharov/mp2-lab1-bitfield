#pragma once
#include <cstddef>

template<bool rev = false>
struct size_iterand final {
private:
    size_t i {};
public:
    constexpr size_iterand() noexcept = default;
    constexpr size_iterand(size_t i) noexcept : i(i) {}
    [[nodiscard]] constexpr bool operator==(size_iterand o) const noexcept { return i == o.i; }
    [[nodiscard]] constexpr bool operator!=(size_iterand o) const noexcept { return i != o.i; }
    [[nodiscard]] constexpr operator size_t () const noexcept { return i; }
    [[nodiscard]] constexpr size_t operator*() const noexcept { return (rev) ? i - 1 : i; }
    [[nodiscard]] constexpr size_iterand& operator++()    noexcept
        { i = (rev) ? i - 1 : i + 1; return *this; }
    [[nodiscard]] constexpr size_t        operator++(int) noexcept
        { size_t old = i; ++*this; return old; }
};

struct indices final {
private:
    size_t len = 0;
public:
    explicit constexpr indices(size_t len) noexcept : len(len) {}
    bool operator==(indices const&) = delete;
    bool operator!=(indices const&) = delete;
    [[nodiscard]] constexpr size_iterand<false>  begin() const noexcept {return 0  ;} // NOLINT
    [[nodiscard]] constexpr size_iterand<false>  end  () const noexcept {return len;}
    [[nodiscard]] constexpr size_iterand<true > rbegin() const noexcept {return len;}
    [[nodiscard]] constexpr size_iterand<true > rend  () const noexcept {return 0  ;} // NOLINT
    [[nodiscard]] constexpr size_t              count () const noexcept {return len;}
};
