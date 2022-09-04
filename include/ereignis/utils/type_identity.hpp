#pragma once

namespace ereignis
{
    template <typename T> struct type_identity
    {
        using type = T;
    };
} // namespace ereignis