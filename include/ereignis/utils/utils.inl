#pragma once

#include "utils.hpp"

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

    template <typename T>
    decltype(auto) forward_ref(std::remove_reference_t<T> &value)
    {
        if constexpr (std::is_lvalue_reference_v<T>)
        {
            return value;
        }
        else
        {
            return static_cast<const T &>(value);
        }
    }
} // namespace ereignis::utils
