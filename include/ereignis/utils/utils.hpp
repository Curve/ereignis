#pragma once

#include <concepts>
#include <type_traits>

namespace ereignis::utils
{
    template <auto T, auto U>
    concept comparable = std::equality_comparable_with<decltype(T), decltype(U)>;

    template <auto I>
    using constant = std::integral_constant<decltype(I), I>;

    template <typename T>
    decltype(auto) forward_ref(std::remove_reference_t<T> &);
} // namespace ereignis::utils

#include "utils.inl"
