#pragma once

#include "event.hpp"
#include "invoker.hpp"

namespace ereignis
{
    template <auto Id, impl::storable Callback>
    void event<Id, Callback>::clear()
    {
        std::lock_guard lock{m_mutex};
        m_callbacks.clear();
    }

    template <auto Id, impl::storable Callback>
    void event<Id, Callback>::remove(std::size_t id)
    {
        std::lock_guard lock{m_mutex};
        m_callbacks.erase(id);
    }

    template <auto Id, impl::storable Callback>
    void event<Id, Callback>::once(callback_type callback)
    {
        std::lock_guard lock{m_mutex};

        auto id = ++m_counter;

        auto handler = [this, id, callback = std::move(callback)]<typename... Ts>(Ts &&...args)
        {
            remove(id);
            return callback(std::forward<Ts>(args)...);
        };

        m_callbacks.emplace(id, std::move(handler));
    }

    template <auto Id, impl::storable Callback>
    std::size_t event<Id, Callback>::add(callback_type callback)
    {
        std::lock_guard lock{m_mutex};

        auto id = ++m_counter;
        m_callbacks.emplace(id, std::move(callback));

        return id;
    }

    template <auto Id, impl::storable Callback>
    template <typename... Ts>
    void event<Id, Callback>::fire(Ts &&...args)
        requires std::is_void_v<result_type> and std::invocable<callback_type, Ts...>
    {
        auto copy = [this]()
        {
            std::lock_guard lock{m_mutex};
            return m_callbacks;
        }();

        for (auto &[_, callback] : copy)
        {
            std::invoke(callback, std::forward<Ts>(args)...);
        }
    }

    template <auto Id, impl::storable Callback>
    template <typename... Ts>
    auto event<Id, Callback>::fire(Ts &&...args)
        requires std::invocable<callback_type, Ts...>
    {
        std::lock_guard lock{m_mutex};
        return invoker{m_callbacks, std::make_tuple(std::forward<Ts>(args)...)};
    }

    template <auto Id, impl::storable Callback>
    template <typename U, typename... Ts>
    std::optional<typename event<Id, Callback>::result_type> event<Id, Callback>::until(U &&value, Ts &&...args)
        requires std::invocable<callback_type, Ts...> and impl::iterable<U, result_type>
    {
        for (auto result : fire(std::forward<Ts>(args)...))
        {
            if (result != value)
            {
                continue;
            }

            return result;
        }

        return std::nullopt;
    }

    template <auto Id, impl::storable Callback>
    template <typename U, typename... Ts>
    std::optional<typename event<Id, Callback>::result_type> event<Id, Callback>::during(U &&value, Ts &&...args)
        requires std::invocable<callback_type, Ts...> and impl::iterable<U, result_type>
    {
        for (auto result : fire(std::forward<Ts>(args)...))
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
