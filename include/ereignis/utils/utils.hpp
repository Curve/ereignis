#pragma once

#include <type_traits>

namespace ereignis::utils
{
    template <auto I>
    using constant = std::integral_constant<decltype(I), I>;

    template <typename T, typename U>
    constexpr bool equals(T, U);
} // namespace ereignis::utils

#include "utils.inl"
