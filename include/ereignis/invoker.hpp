#pragma once
#include <map>
#include <mutex>
#include <cstdint>

namespace ereignis
{
    template <typename Type, typename Callback, typename... Params> class invoker
    {
        using map_t = std::map<std::uint64_t, Callback>;

        using const_iterator_t = typename map_t::const_iterator;
        using iterator_t = typename map_t::iterator;

      private:
        std::lock_guard<std::mutex> m_guard;
        map_t &m_map;

      private:
        std::tuple<Params...> m_params;

      public:
        invoker(std::mutex &, map_t &, std::tuple<Params...> &&);

      public:
        auto begin();
        auto begin() const;

      public:
        auto end();
        auto end() const;
    };
} // namespace ereignis

#include "invoker.inl"