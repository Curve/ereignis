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
    std::size_t event<Id, Callback>::add(T &&callback)
        requires std::constructible_from<callback_t, T>
    {
        std::lock_guard lock(m_mutex);

        auto id = ++m_counter;
        m_callbacks.emplace(id, std::forward<T>(callback));

        return id;
    }

    template <auto Id, callback Callback>
    template <typename T>
    void event<Id, Callback>::once(T &&callback)
        requires std::constructible_from<callback_t, T>
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
    auto event<Id, Callback>::fire(T &&...args) const
        requires valid_arguments<callback_t, T...>
    {
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
    std::optional<typename event<Id, Callback>::result_t> event<Id, Callback>::until(U &&value, T &&...args) const
        requires valid_arguments<callback_t, T...> and std::equality_comparable_with<result_t, U>
    {
        for (auto &&result : fire(std::forward<T>(args)...))
        {
            if (result != value)
            {
                continue;
            }

            return result;
        }

        return std::nullopt;
    }

    template <auto Id, callback Callback>
    template <typename U, typename... T>
    std::optional<typename event<Id, Callback>::result_t> event<Id, Callback>::during(U &&value, T &&...args) const
        requires valid_arguments<callback_t, T...> and std::equality_comparable_with<result_t, U>
    {
        for (auto &&result : fire(std::forward<T>(args)...))
        {
            if (result == value)
            {
                continue;
            }

            return result;
        }

        return std::nullopt;
    }
} // namespace ereignis
