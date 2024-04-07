#pragma once

#include "event.hpp"
#include "invoker.hpp"

namespace ereignis
{
    template <auto Id, callback Callback>
    void event<Id, Callback>::clear()
    {
        std::lock_guard lock(m_mutex);
        m_callbacks.clear();
    }

    template <auto Id, callback Callback>
    void event<Id, Callback>::remove(std::size_t id)
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
        m_callbacks.emplace(id, std::forward<T>(callback));

        return id;
    }

    template <auto Id, callback Callback>
    template <typename T>
        requires valid_callback<Callback, T>
    void event<Id, Callback>::once(T &&callback)
    {
        std::lock_guard lock(m_mutex);

        auto id = ++m_counter;

        auto handler = [this, id, callback = std::forward<T>(callback)]<typename... A>(A &&...args)
        {
            remove(id);
            return callback(std::forward<A>(args)...);
        };

        m_callbacks.emplace(id, std::move(handler));
    }

    template <auto Id, callback Callback>
    template <typename... T>
        requires valid_arguments<Callback, T...>
    auto event<Id, Callback>::fire(T &&...args) const
    {
        using result_t = std::invoke_result_t<Callback, T...>;

        if constexpr (std::is_void_v<result_t>)
        {
            const auto copy = m_callbacks;

            for (const auto &[key, callback] : copy)
            {
                callback(args...);
            }
        }
        else
        {
            return invoker(m_callbacks, std::make_tuple(std::forward<T>(args)...));
        }
    }

    template <auto Id, callback Callback>
    template <typename U, typename... T>
        requires valid_arguments<Callback, T...> and
                 std::equality_comparable_with<std::invoke_result_t<Callback, T...>, U>
    std::optional<std::invoke_result_t<Callback, T...>> event<Id, Callback>::until(U &&result, T &&...args) const
    {
        const auto copy = m_callbacks;

        for (const auto &[key, callback] : copy)
        {
            if (auto res = callback(args...); res == result)
            {
                return res;
            }
        }

        return std::nullopt;
    }
} // namespace ereignis
