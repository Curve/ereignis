#pragma once

#include <map>
#include <mutex>
#include <optional>

#include <concepts>
#include <functional>

namespace ereignis
{
    namespace impl
    {
        template <typename T>
        concept storable = requires(T callback) {
            { std::function{callback} };
        };

        template <typename T, typename R>
        concept iterable = requires() {
            requires not std::is_void_v<R>;
            requires std::equality_comparable_with<R, T>;
        };
    } // namespace impl

    template <auto Id, impl::storable Callback>
    struct event
    {
        static constexpr auto id = Id;
        using callback_type      = std::function<Callback>;
        using result_type        = callback_type::result_type;

      private:
        std::mutex m_mutex;
        std::size_t m_counter{0};
        std::map<std::size_t, callback_type> m_callbacks;

      public:
        void clear();
        void remove(std::size_t id);

      public:
        void once(callback_type callback);
        std::size_t add(callback_type callback);

      public:
        template <typename... Ts>
        void fire(Ts &&...args)
            requires std::is_void_v<result_type> and std::invocable<callback_type, Ts...>;

        template <typename... Ts>
        [[nodiscard]] auto fire(Ts &&...args)
            requires std::invocable<callback_type, Ts...>;

      public:
        template <typename U, typename... Ts>
        std::optional<result_type> until(U &&value, Ts &&...args)
            requires std::invocable<callback_type, Ts...> and impl::iterable<U, result_type>;

        template <typename U, typename... Ts>
        std::optional<result_type> during(U &&value, Ts &&...args)
            requires std::invocable<callback_type, Ts...> and impl::iterable<U, result_type>;
    };
} // namespace ereignis

#include "event.inl"
