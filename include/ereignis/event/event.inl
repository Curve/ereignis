#pragma once

#include "event.hpp"
#include "../utils/utils.hpp"

#include <ranges>

namespace ereignis
{
    template <typename T>
    struct impl::is_event : std::false_type
    {
    };

    template <auto Id, typename Signature>
    struct impl::is_event<event<Id, Signature>> : std::true_type
    {
    };

    template <auto Id, typename Signature>
    auto event<Id, Signature>::copy()
    {
        auto lock = std::lock_guard{m_mutex};
        return m_callbacks | std::views::values | std::ranges::to<std::vector>();
    }

    template <auto Id, typename Signature>
    void event<Id, Signature>::once(callback cb)
    {
        auto lock = std::lock_guard{m_mutex};
        auto id   = m_counter++;

        auto wrapped = std::make_shared<callback>();
        *wrapped     = [this, id, wrapped, cb = std::move(cb)]<typename... Ts>(Ts &&...args) mutable
        {
            auto guard = std::move(wrapped);
            remove(id);

            return std::invoke(cb, std::forward<Ts>(args)...);
        };

        m_callbacks.emplace(id, std::move(wrapped));
    }

    template <auto Id, typename Signature>
    std::size_t event<Id, Signature>::add(callback cb)
    {
        auto lock = std::lock_guard{m_mutex};
        auto id   = m_counter++;

        m_callbacks.emplace(id, std::make_shared<callback>(std::move(cb)));
        return id;
    }

    template <auto Id, typename Signature>
    void event<Id, Signature>::on_clear(clear_callback cb)
    {
        auto lock        = std::lock_guard{m_clear_mutex};
        m_clear_callback = std::move(cb);
    }

    template <auto Id, typename Signature>
    void event<Id, Signature>::clear()
    {
        auto lock = std::unique_lock{m_mutex};
        m_callbacks.clear();

        lock = std::unique_lock{m_clear_mutex};

        if (!m_clear_callback)
        {
            return;
        }

        std::invoke(m_clear_callback);
    }

    template <auto Id, typename Signature>
    void event<Id, Signature>::remove(std::size_t id)
    {
        auto lock = std::lock_guard{m_mutex};
        m_callbacks.erase(id);
    }

    template <auto Id, typename Signature>
    template <typename... Ts>
    void event<Id, Signature>::fire(Ts &&...args) // NOLINT(*-std-forward)
        requires(std::is_void_v<result>)
    {
        for (const auto &callback : copy())
        {
            std::invoke(*callback, utils::forward_ref<Ts>(args)...);
        }
    }

    template <auto Id, typename Signature>
    template <typename... Ts>
    auto event<Id, Signature>::fire(Ts &&...args) -> coco::generator<result> // NOLINT(*-std-forward)
        requires(not std::is_void_v<result>)
    {
        for (const auto &callback : copy())
        {
            co_yield std::invoke(*callback, utils::forward_ref<Ts>(args)...);
        }
    }
} // namespace ereignis
