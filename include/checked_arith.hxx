#pragma once
#include <stdexcept>
#include <type_traits>
template<typename T>
[[nodiscard]] constexpr bool checked_add(T& dst, T a, T b) noexcept
    { return __builtin_add_overflow(a, b, &dst); }
template<typename T>
[[nodiscard]] constexpr bool checked_sub(T& dst, T a, T b) noexcept
    { return __builtin_sub_overflow(a, b, &dst); }
template<typename T>
[[nodiscard]] constexpr bool checked_mul(T& dst, T a, T b) noexcept
    { return __builtin_mul_overflow(a, b, &dst); }

template<typename T, typename Exc = std::domain_error>
[[nodiscard]] constexpr T throwing_add(T a, T b) {
    std::remove_const_t<T> rslt;
    if (checked_add(rslt, a, b)) throw Exc();
    return rslt;
}
template<typename T, typename Exc = std::domain_error>
[[nodiscard]] constexpr T throwing_sub(T a, T b) {
    std::remove_const_t<T> rslt;
    if (checked_sub(rslt, a, b)) throw Exc();
    return rslt;
}
template<typename T, typename Exc = std::domain_error>
[[nodiscard]] constexpr T throwing_mul(T a, T b) {
    std::remove_const_t<T> rslt;
    if (checked_mul(rslt, a, b)) throw Exc();
    return rslt;
}
