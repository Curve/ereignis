#pragma once
#include "event.hpp"

namespace ereignis
{
    template <auto Id, callback Callback> void event<Id, Callback>::clear()
    {
        std::lock_guard lock(m_mutex);
        m_callbacks.clear();
    }

    template <auto Id, callback Callback> void event<Id, Callback>::remove(std::size_t id)
    {
        std::lock_guard lock(m_mutex);
        m_callbacks.erase(id);
    }

    template <auto Id, callback Callback>
    template <typename T>
        requires valid_callback<Callback, T>
    std::size_t event<Id, Callback>::add(T &&callback)
    {
        std::lock_guard lock(m_mutex);

        auto id = ++m_counter;
        m_callbacks.emplace(id, std::forward<decltype(callback)>(callback));

        return id;
    }

    template <auto Id, callback Callback>
    template <typename... T>
        requires valid_arguments<Callback, T...>
    auto event<Id, Callback>::fire(T &&...args)
    {
        using return_t = std::invoke_result_t<Callback, T...>;

        if constexpr (std::is_same_v<return_t, void>)
        {
            for (const auto &[key, callback] : m_callbacks)
            {
                callback(args...);
            }
        }
        else
        {
            return invoker(m_callbacks, std::make_tuple(args...));
        }
    }

    template <auto Id, callback Callback>
    template <typename... T>
        requires valid_arguments<Callback, T...>
    auto event<Id, Callback>::fire(T &&...args) const
    {
        return invoker(m_callbacks, std::make_tuple(args...));
    }
} // namespace ereignis