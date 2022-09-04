#pragma once
#include "event.hpp"
#include <type_traits>
#include "utils/type_traits.hpp"

namespace ereignis
{
    template <auto Id, typename Callback> void event<Id, Callback>::clear()
    {
        std::lock_guard guard(m_mutex);
        m_callbacks.clear();
    }

    template <auto Id, typename Callback> void event<Id, Callback>::remove(std::uint64_t id)
    {
        std::lock_guard guard(m_mutex);
        m_callbacks.erase(id);
    }

    template <auto Id, typename Callback> std::uint64_t event<Id, Callback>::add(callback_t &&callback)
    {
        std::lock_guard guard(m_mutex);

        auto id = ++m_counter;
        m_callbacks.emplace(id, std::move(callback));

        return id;
    }

    template <auto Id, typename Callback> //
    template <typename... Params>
    auto event<Id, Callback>::fire(Params &&...params)
    {
        auto parameters = std::tuple<remove_const_ref_t<Params>...>{std::forward<Params>(params)...};

        if constexpr (std::is_same_v<result_t, void>)
        {
            auto invoker = ereignis::invoker<result_t, callback_t, remove_const_ref_t<Params>...>(m_mutex, m_callbacks, std::move(parameters));
            for (auto it = invoker.begin(); it != invoker.end(); ++it)
            {
                (*it);
            }
        }
        else
        {
            return invoker<result_t, callback_t, remove_const_ref_t<Params>...>(m_mutex, m_callbacks, std::move(parameters));
        }
    }

    template <auto Id, typename Callback> //
    template <typename... Params>
    auto event<Id, Callback>::fire(Params &&...params) const
    {
        auto parameters = std::tuple<remove_const_ref_t<Params>...>{std::forward<Params>(params)...};

        if constexpr (std::is_same_v<result_t, void>)
        {
            auto invoker = ereignis::invoker<result_t, callback_t, remove_const_ref_t<Params>...>(m_mutex, m_callbacks, std::move(parameters));
            for (auto it = invoker.begin(); it != invoker.end(); ++it)
            {
                (*it);
            }
        }
        else
        {
            return invoker<result_t, callback_t, remove_const_ref_t<Params>...>(m_mutex, m_callbacks, std::move(parameters));
        }
    }
} // namespace ereignis