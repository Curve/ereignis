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
        using callback           = std::function<Callback>;
        using result             = callback::result_type;

      private:
        using clear_callback = std::function<void()>;

      private:
        std::mutex m_mutex;
        std::size_t m_counter{0};
        std::map<std::size_t, callback> m_callbacks;

      private:
        std::mutex m_clear_mutex;
        clear_callback m_clear_callback;

      private:
        void on_clear();
        [[nodiscard]] auto callbacks();

      public:
        void clear();
        void remove(std::size_t id);

      public:
        [[nodiscard]] bool empty();
        void on_clear(clear_callback);

      public:
        void once(callback callback);
        std::size_t add(callback callback);

      public:
        template <typename... Ts>
        void fire(Ts &&...args)
            requires std::is_void_v<result> and std::invocable<callback, Ts...>;

        template <typename... Ts>
        [[nodiscard]] auto fire(Ts &&...args)
            requires std::invocable<callback, Ts...>;

      public:
        template <typename U, typename... Ts>
        std::optional<result> until(U &&value, Ts &&...args)
            requires std::invocable<callback, Ts...> and impl::iterable<U, result>;

        template <typename U, typename... Ts>
        std::optional<result> during(U &&value, Ts &&...args)
            requires std::invocable<callback, Ts...> and impl::iterable<U, result>;
    };
} // namespace ereignis

#include "event.inl"
