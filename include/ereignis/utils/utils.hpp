#pragma once

#include <concepts>
#include <type_traits>

namespace ereignis::utils
{
    template <auto I>
    using constant = std::integral_constant<decltype(I), I>;

    template <typename T, typename U>
    constexpr bool equals(T, U);

    template <typename T>
    decltype(auto) forward_ref(std::remove_reference_t<T> &);
} // namespace ereignis::utils

#include "utils.inl"
