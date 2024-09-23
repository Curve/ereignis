#pragma once

#include "event.hpp"
#include "invoker.hpp"

namespace ereignis
{
    template <auto Id, impl::storable Callback>
    void event<Id, Callback>::on_clear()
    {
        std::lock_guard lock{m_clear_mutex};

        if (!m_clear_callback)
        {
            return;
        }

        std::invoke(m_clear_callback);
    }

    template <auto Id, impl::storable Callback>
    auto event<Id, Callback>::callbacks()
    {
        std::lock_guard lock{m_mutex};
        return m_callbacks;
    }

    template <auto Id, impl::storable Callback>
    void event<Id, Callback>::clear()
    {
        {
            std::lock_guard lock{m_mutex};
            m_callbacks.clear();
        }
        on_clear();
    }

    template <auto Id, impl::storable Callback>
    void event<Id, Callback>::remove(std::size_t id)
    {
        {
            std::lock_guard lock{m_mutex};
            m_callbacks.erase(id);

            if (!m_callbacks.empty())
            {
                return;
            }
        }
        on_clear();
    }

    template <auto Id, impl::storable Callback>
    bool event<Id, Callback>::empty()
    {
        std::lock_guard lock{m_mutex};
        return m_callbacks.empty();
    }

    template <auto Id, impl::storable Callback>
    void event<Id, Callback>::on_clear(clear_callback callback)
    {
        std::lock_guard lock{m_clear_mutex};
        m_clear_callback = std::move(callback);
    }

    template <auto Id, impl::storable Callback>
    void event<Id, Callback>::once(callback callback)
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
    std::size_t event<Id, Callback>::add(callback callback)
    {
        std::lock_guard lock{m_mutex};

        auto id = ++m_counter;
        m_callbacks.emplace(id, std::move(callback));

        return id;
    }

    template <auto Id, impl::storable Callback>
    template <typename... Ts>
    void event<Id, Callback>::fire(Ts &&...args)
        requires std::is_void_v<result> and std::invocable<callback, Ts...>
    {
        for (auto &[_, callback] : callbacks())
        {
            std::invoke(callback, args...);
        }
    }

    template <auto Id, impl::storable Callback>
    template <typename... Ts>
    auto event<Id, Callback>::fire(Ts &&...args)
        requires std::invocable<callback, Ts...>
    {
        std::lock_guard lock{m_mutex};
        return invoker{m_callbacks, std::make_tuple(std::forward<Ts>(args)...)};
    }

    template <auto Id, impl::storable Callback>
    template <typename U, typename... Ts>
    std::optional<typename event<Id, Callback>::result> event<Id, Callback>::until(U &&value, Ts &&...args)
        requires std::invocable<callback, Ts...> and impl::iterable<U, result>
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
    std::optional<typename event<Id, Callback>::result> event<Id, Callback>::during(U &&value, Ts &&...args)
        requires std::invocable<callback, Ts...> and impl::iterable<U, result>
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
