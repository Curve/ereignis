#pragma once
#include <map>
#include <mutex>
#include <cstdint>

namespace ereignis
{
    template <typename Type, typename Callback, typename... Params> class iterator
    {
        using iterator_t = typename std::map<std::uint64_t, Callback>::iterator;

      private:
        iterator_t m_iterator;

      private:
        std::tuple<Params...> &m_params;

      public:
        iterator(std::tuple<Params...> &, iterator_t iterator);

      public:
        Type operator*();
        Type operator*() const;

      public:
        iterator &operator++();
        iterator operator++() const;

      public:
        bool operator!=(const iterator &) const;
    };
} // namespace ereignis

#include "iterator.inl"