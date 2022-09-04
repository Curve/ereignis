#pragma once
#include "invoker.hpp"
#include "iterator.hpp"

namespace ereignis
{
    template <typename Type, typename Callback, typename... Params>
    invoker<Type, Callback, Params...>::invoker(std::mutex &mutex, map_t &map, std::tuple<Params...> &&params) : m_guard(mutex), m_map(map), m_params(std::move(params))
    {
    }

    template <typename Type, typename Callback, typename... Params> auto invoker<Type, Callback, Params...>::begin()
    {
        return iterator<Type, Callback, Params...>(m_params, m_map.begin());
    }

    template <typename Type, typename Callback, typename... Params> auto invoker<Type, Callback, Params...>::begin() const
    {
        return iterator<Type, Callback, Params...>(m_params, m_map.begin());
    }

    template <typename Type, typename Callback, typename... Params> auto invoker<Type, Callback, Params...>::end()
    {
        return iterator<Type, Callback, Params...>(m_params, m_map.end());
    }

    template <typename Type, typename Callback, typename... Params> auto invoker<Type, Callback, Params...>::end() const
    {
        return iterator<Type, Callback, Params...>(m_params, m_map.end());
    }
} // namespace ereignis