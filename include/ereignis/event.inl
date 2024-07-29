#pragma once

#include "event.hpp"

namespace ereignis
{
    template <auto Id, detail::callback Callback>
    void event<Id, Callback>::clear()
    {
        auto locked = m_callbacks.write();
        locked->clear();
    }

    template <auto Id, detail::callback Callback>
    void event<Id, Callback>::remove(std::size_t id)
    {
        auto locked = m_callbacks.write();
        locked->erase(id);
    }

    template <auto Id, detail::callback Callback>
    template <typename T>
    std::size_t event<Id, Callback>::add(T &&callback)
        requires std::constructible_from<callback_t, T>
    {
        auto locked = m_callbacks.write();

        auto id = ++m_counter;
        locked->emplace(id, std::forward<T>(callback));

        return id;
    }

    template <auto Id, detail::callback Callback>
    template <typename T>
    void event<Id, Callback>::once(T &&callback)
        requires std::constructible_from<callback_t, T>
    {
        auto locked = m_callbacks.write();
        auto id     = ++m_counter;

        auto handler = [this, id, callback = std::forward<T>(callback)]<typename... A>(A &&...args)
        {
            remove(id);
            return callback(std::forward<A>(args)...);
        };

        locked->emplace(id, std::move(handler));
    }

    template <auto Id, detail::callback Callback>
    template <typename... T>
    std::generator<typename event<Id, Callback>::result_t> event<Id, Callback>::fire(T &&...args) const
        requires detail::valid_arguments<callback_t, T...>
    {
        auto callbacks = m_callbacks.copy();

        for (const auto &[key, callback] : callbacks)
        {
            co_yield std::invoke(callback, std::forward<T>(args)...);
        }
    }

    template <auto Id, detail::callback Callback>
    template <typename... T>
    void event<Id, Callback>::fire(T &&...args) const
        requires detail::valid_arguments<callback_t, T...> and std::is_void_v<result_t>
    {
        auto callbacks = m_callbacks.copy();

        for (const auto &[key, callback] : callbacks)
        {
            std::invoke(callback, std::forward<T>(args)...);
        }
    }

    template <auto Id, detail::callback Callback>
    template <typename U, typename... T>
    std::optional<typename event<Id, Callback>::result_t> event<Id, Callback>::until(U &&value, T &&...args) const
        requires detail::valid_arguments<callback_t, T...> and std::equality_comparable_with<result_t, U>
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

    template <auto Id, detail::callback Callback>
    template <typename U, typename... T>
    std::optional<typename event<Id, Callback>::result_t> event<Id, Callback>::during(U &&value, T &&...args) const
        requires detail::valid_arguments<callback_t, T...> and std::equality_comparable_with<result_t, U>
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
