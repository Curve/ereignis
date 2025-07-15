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
    void event<Id, R(Ts...)>::cleared()
    {
        auto lock = std::lock_guard{m_clear_mutex};

        if (!m_clear_callback)
        {
            return;
        }

        std::invoke(m_clear_callback);
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
    std::size_t event<Id, R(Ts...)>::add(function cb)
    {
        return add({.func = std::move(cb)});
    }

    template <auto Id, typename R, typename... Ts>
    void event<Id, R(Ts...)>::once(callback cb)
    {
        auto lock = std::lock_guard{m_mutex};
        auto id   = m_counter++;

        cb.func = [this, state = std::make_tuple(id, std::move(cb.func))]<typename... Us>(Us &&...args) mutable
        {
            auto [id, func] = std::move(state);

            remove(id);
            return std::invoke(func, std::forward<Us>(args)...);
        };

        m_callbacks.emplace(id, std::make_shared<callback>(std::move(cb)));
    }

    template <auto Id, typename R, typename... Ts>
    void event<Id, R(Ts...)>::once(function cb)
    {
        once({.func = std::move(cb)});
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
        auto lock = std::lock_guard{m_mutex};

        for (auto it = m_callbacks.begin(); it != m_callbacks.end();)
        {
            const auto &[id, callback] = *it;

            if (!callback->clearable)
            {
                ++it;
                continue;
            }

            it = m_callbacks.erase(it);
        }

        if (!m_callbacks.empty())
        {
            return;
        }

        cleared();
    }

    template <auto Id, typename R, typename... Ts>
    void event<Id, R(Ts...)>::remove(std::size_t id)
    {
        auto lock = std::lock_guard{m_mutex};
        m_callbacks.erase(id);

        if (!m_callbacks.empty())
        {
            return;
        }

        cleared();
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
            std::invoke(callback->func, args...);
        }
    }

    template <auto Id, typename R, typename... Ts>
    auto event<Id, R(Ts...)>::fire(Ts... args) -> coco::generator<result>
        requires(not std::is_void_v<result>)
    {
        for (const auto &callback : copy())
        {
            co_yield std::invoke(callback->func, args...);
        }
    }
} // namespace ereignis
