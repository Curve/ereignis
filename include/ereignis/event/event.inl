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

    template <typename R, typename... Ts>
    struct listener<R(Ts...), true>
    {
        using callback = std::move_only_function<R(Ts...)>;

      public:
        callback func;
        bool clearable{true};
    };

    template <typename R, typename... Ts>
    struct listener<R(Ts...), false> : listener<R(Ts...), true>
    {
        using base = listener<R(Ts...), true>;

      public:
        listener(base value) : base(std::move(value)) {}

      public:
        template <typename T>
            requires std::same_as<std::invoke_result_t<T, Ts...>, R>
        listener(T &&func) : base(std::forward<T>(func))
        {
        }
    };

    template <auto Id, typename R, typename... Ts>
    auto event<Id, R(Ts...)>::copy()
    {
        auto lock = std::lock_guard{m_mutex};
        return m_listeners | std::views::values | std::ranges::to<std::vector>();
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
    std::size_t event<Id, R(Ts...)>::add(listener value)
    {
        const auto id = m_counter++;
        update(id, std::move(value));

        return id;
    }

    template <auto Id, typename R, typename... Ts>
    void event<Id, R(Ts...)>::update(std::size_t id, listener value)
    {
        auto lock = std::lock_guard{m_mutex};
        m_listeners.insert_or_assign(id, std::make_shared<listener>(std::move(value)));
    }

    template <auto Id, typename R, typename... Ts>
    void event<Id, R(Ts...)>::once(listener::callback cb)
    {
        const auto id = m_counter++;

        auto wrapper = [this, state = std::make_tuple(id, std::move(cb))]<typename... Us>(Us &&...args) mutable
        {
            auto [id, cb] = std::move(state);

            remove(id);
            return std::invoke(cb, std::forward<Us>(args)...);
        };

        update(id, std::move(wrapper));
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

        for (auto it = m_listeners.begin(); it != m_listeners.end();)
        {
            const auto &[id, listener] = *it;

            if (!listener->clearable)
            {
                ++it;
                continue;
            }

            it = m_listeners.erase(it);
        }

        if (!m_listeners.empty())
        {
            return;
        }

        cleared();
    }

    template <auto Id, typename R, typename... Ts>
    void event<Id, R(Ts...)>::remove(std::size_t id)
    {
        auto lock = std::lock_guard{m_mutex};
        m_listeners.erase(id);

        if (!m_listeners.empty())
        {
            return;
        }

        cleared();
    }

    template <auto Id, typename R, typename... Ts>
    bool event<Id, R(Ts...)>::empty()
    {
        auto lock = std::lock_guard{m_mutex};
        return m_listeners.empty();
    }

    template <auto Id, typename R, typename... Ts>
    void event<Id, R(Ts...)>::fire(Ts... args)
        requires(std::is_void_v<result>)
    {
        for (const auto &listener : copy())
        {
            std::invoke(listener->func, args...);
        }
    }

    template <auto Id, typename R, typename... Ts>
    auto event<Id, R(Ts...)>::fire(Ts... args) -> coco::generator<result>
        requires(not std::is_void_v<result>)
    {
        for (const auto &listener : copy())
        {
            co_yield std::invoke(listener->func, args...);
        }
    }
} // namespace ereignis
