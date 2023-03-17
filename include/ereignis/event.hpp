#pragma once
#include "invoker.hpp"

#include <map>
#include <mutex>
#include <atomic>
#include <functional>

namespace ereignis
{
    template <typename T>
    concept callback = requires(T t) { std::function{t}; };

    template <typename Callback, typename... T>
    concept valid_arguments = requires(Callback &callback, T &&...t) { callback(std::forward<T>(t)...); };

    template <auto Id, callback Callback> class event
    {
      private:
        std::mutex m_mutex;
        std::atomic_size_t m_counter{0};
        std::map<std::size_t, std::function<Callback>> m_callbacks;

      public:
        void clear();

      public:
        void remove(std::size_t id);
        std::size_t add(callback auto &&callback);

      public:
        template <typename... T>
            requires valid_arguments<Callback, T...>
        auto fire(T &&...args);

        template <typename... T>
            requires valid_arguments<Callback, T...>
        auto fire(T &&...args) const;

      public:
        static constexpr auto id = Id;
    };
} // namespace ereignis

#include "event.inl"