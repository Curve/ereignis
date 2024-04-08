#pragma once

#include <map>
#include <mutex>
#include <atomic>
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
            requires std::constructible_from<callback_t, T>
        std::size_t add(T &&callback);

        template <typename T>
            requires std::constructible_from<callback_t, T>
        void once(T &&callback);

      public:
        template <typename... T>
            requires valid_arguments<Callback, T...>
        auto fire(T &&...args) const;

        template <typename U, typename... T>
            requires valid_arguments<Callback, T...> and std::equality_comparable_with<result_t, U>
        auto until(U &&value, T &&...args) const;

      public:
        static constexpr auto id = Id;
    };
} // namespace ereignis

#include "event.inl"
