#pragma once

#include <map>
#include <atomic>

#include <optional>
#include <generator>
#include <functional>

#include <lockpp/lock.hpp>

namespace ereignis
{
    namespace detail
    {
        template <typename T>
        concept callback = requires(T t) { std::function{t}; };

        template <typename Callback, typename... T>
        concept valid_arguments = requires(Callback callback, T &&...args) { callback(std::forward<T>(args)...); };
    } // namespace detail

    template <auto Id, detail::callback Callback>
    class event
    {
        using callback_t = std::function<Callback>;
        using result_t   = callback_t::result_type;

      private:
        std::atomic_size_t m_counter{0};
        lockpp::lock<std::map<std::size_t, callback_t>> m_callbacks;

      public:
        void clear();

      public:
        void remove(std::size_t id);

      public:
        template <typename T>
        std::size_t add(T &&callback)
            requires std::constructible_from<callback_t, T>;

        template <typename T>
        void once(T &&callback)
            requires std::constructible_from<callback_t, T>;

      public:
        template <typename... T>
        std::generator<result_t> fire(T &&...args) const
            requires detail::valid_arguments<callback_t, T...>;

        template <typename... T>
        void fire(T &&...args) const
            requires detail::valid_arguments<callback_t, T...> and std::is_void_v<result_t>;

      public:
        template <typename U, typename... T>
        std::optional<result_t> until(U &&value, T &&...args) const
            requires detail::valid_arguments<callback_t, T...> and std::equality_comparable_with<result_t, U>;

        template <typename U, typename... T>
        std::optional<result_t> during(U &&value, T &&...args) const
            requires detail::valid_arguments<callback_t, T...> and std::equality_comparable_with<result_t, U>;

      public:
        static constexpr auto id = Id;
    };
} // namespace ereignis

#include "event.inl"
