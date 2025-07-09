#pragma once

#include "event.hpp"

#include <ranges>

namespace ereignis
{
    template <typename T>
    struct impl::is_event : std::false_type
    {
    };

    template <auto Id, typename R, typename... Ts>
    struct impl::is_event<event<Id, R(Ts...)>> : std::true_type
    {
    };

    template <typename... Ts>
    struct impl::await_result
    {
        using type = std::tuple<Ts...>;
    };

    template <typename T>
    struct impl::await_result<T>
    {
        using type = T;
    };

    template <>
    struct impl::await_result<>
    {
        using type = void;
    };

    template <typename Result, typename... Ts>
    struct impl::await_return : std::false_type
    {
    };

    template <>
    struct impl::await_return<void> : std::true_type
    {
    };

    template <typename Result, typename... Ts>
        requires(sizeof...(Ts) > 0 && std::constructible_from<Result, Ts...>)
    struct impl::await_return<Result, Ts...> : std::true_type
    {
    };

    template <auto Id, typename R, typename... Ts>
    auto event<Id, R(Ts...)>::copy()
    {
        auto lock = std::lock_guard{m_mutex};
        return m_callbacks | std::views::values | std::ranges::to<std::vector>();
    }

    template <auto Id, typename R, typename... Ts>
    void event<Id, R(Ts...)>::once(callback cb)
    {
        auto lock = std::lock_guard{m_mutex};
        auto id   = m_counter++;

        auto wrapped = [this, state = std::make_tuple(id, std::move(cb))]<typename... Us>(Us &&...args) mutable
        {
            auto [id, cb] = std::move(state);
            remove(id);
            return std::invoke(cb, std::forward<Us>(args)...);
        };

        m_callbacks.emplace(id, std::make_shared<callback>(std::move(wrapped)));
    }

    template <auto Id, typename R, typename... Ts>
    std::size_t event<Id, R(Ts...)>::add(callback cb)
    {
        auto lock = std::lock_guard{m_mutex};
        auto id   = m_counter++;

        m_callbacks.emplace(id, std::make_shared<callback>(std::move(cb)));
        return id;
    }

    template <auto Id, typename R, typename... Ts>
    template <typename... Rs>
    auto event<Id, R(Ts...)>::await(Rs &&...res) -> future
        requires AwaitReturn<result, Rs...>
    {
        auto promise = coco::promise<await_result>{};
        auto future  = promise.get_future();

        once(
            [promise = std::move(promise), ... res = std::forward<Rs>(res)]<typename... Us>(Us &&...args) mutable
            {
                if constexpr (std::is_void_v<await_result>)
                {
                    promise.set_value();
                }
                else
                {
                    promise.set_value(await_result{std::forward<Us>(args)...});
                }

                if constexpr (!std::is_void_v<result>)
                {
                    return result{std::forward<Rs>(res)...};
                }
                else
                {
                    (static_cast<void>(res), ...);
                }
            });

        return future;
    }

    template <auto Id, typename R, typename... Ts>
    void event<Id, R(Ts...)>::on_clear(clear_callback cb)
    {
        auto lock        = std::lock_guard{m_clear_mutex};
        m_clear_callback = std::move(cb);
    }

    template <auto Id, typename R, typename... Ts>
    void event<Id, R(Ts...)>::clear()
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

    template <auto Id, typename R, typename... Ts>
    void event<Id, R(Ts...)>::remove(std::size_t id)
    {
        auto lock = std::lock_guard{m_mutex};
        m_callbacks.erase(id);
    }

    template <auto Id, typename R, typename... Ts>
    bool event<Id, R(Ts...)>::empty()
    {
        auto lock = std::lock_guard{m_mutex};
        return m_callbacks.empty();
    }

    template <auto Id, typename R, typename... Ts>
    void event<Id, R(Ts...)>::fire(Ts... args)
        requires(std::is_void_v<result>)
    {
        for (const auto &callback : copy())
        {
            std::invoke(*callback, args...);
        }
    }

    template <auto Id, typename R, typename... Ts>
    auto event<Id, R(Ts...)>::fire(Ts... args) -> coco::generator<result>
        requires(not std::is_void_v<result>)
    {
        for (const auto &callback : copy())
        {
            co_yield std::invoke(*callback, args...);
        }
    }
} // namespace ereignis
