#pragma once

#include <map>
#include <mutex>
#include <atomic>

#include <optional>
#include <functional>

namespace ereignis
{
    template <typename T>
    concept callback = requires(T t) { std::function{t}; };

    template <typename Callback, typename... T>
    concept valid_arguments = requires(Callback callback, T &&...args) { callback(std::forward<T>(args)...); };

    template <auto Id, callback Callback>
    class event
    {
        using callback_t = std::function<Callback>;
        using result_t   = callback_t::result_type;

      private:
        std::mutex m_mutex;
        std::atomic_size_t m_counter{0};
        std::map<std::size_t, callback_t> m_callbacks;

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
        auto fire(T &&...args) const
            requires valid_arguments<callback_t, T...>;

        template <typename U, typename... T>
        std::optional<result_t> until(U &&value, T &&...args) const
            requires valid_arguments<callback_t, T...> and std::equality_comparable_with<result_t, U>;

        template <typename U, typename... T>
        std::optional<result_t> during(U &&value, T &&...args) const
            requires valid_arguments<callback_t, T...> and std::equality_comparable_with<result_t, U>;

      public:
        static constexpr auto id = Id;
    };
} // namespace ereignis

#include "event.inl"
