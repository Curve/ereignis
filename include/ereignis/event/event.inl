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

    template <typename T>
    struct impl::result
    {
        T value;

      public:
        struct invalid
        {
        };

        static constexpr auto empty = invalid{};
    };

    template <>
    struct impl::result<void>
    {
        result(std::monostate) {}

      public:
        static constexpr auto empty = std::monostate{};
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
    auto event<Id, R(Ts...)>::await(impl::result<result> res) -> future
    {
        auto promise = coco::promise<await_result>{};
        auto future  = promise.get_future();

        once(
            [promise = std::move(promise), res = std::move(res)]<typename... Us>(Us &&...args) mutable
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
                    return std::move(res.value);
                }
                else
                {
                    static_cast<void>(res);
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
    template <typename... Us>
    void event<Id, R(Ts...)>::fire(Us &&...args) // NOLINT(*-std-forward)
        requires(std::invocable<callback, Us...> and std::is_void_v<result>)
    {
        for (const auto &callback : copy())
        {
            std::invoke(*callback, utils::forward_ref<Us>(args)...);
        }
    }

    template <auto Id, typename R, typename... Ts>
    template <typename... Us>
    auto event<Id, R(Ts...)>::fire(Us &&...args) -> coco::generator<result> // NOLINT(*-std-forward)
        requires(std::invocable<callback, Us...> and not std::is_void_v<result>)
    {
        for (const auto &callback : copy())
        {
            co_yield std::invoke(*callback, utils::forward_ref<Us>(args)...);
        }
    }
} // namespace ereignis
