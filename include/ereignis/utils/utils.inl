#pragma once

#include "utils.hpp"

#include <concepts>

namespace ereignis::utils
{
    template <typename T, typename U>
    constexpr bool equals(T left, U right)
    {
        if constexpr (std::equality_comparable_with<T, U>)
        {
            return left == right;
        }
        else
        {
            return false;
        }
    }
} // namespace ereignis::utils
