#pragma once
#include "invoker.hpp"

#include <map>
#include <mutex>
#include <atomic>
#include <cstdint>
#include <functional>

namespace ereignis
{
    template <auto Id, typename Callback> struct event
    {
        static constexpr auto id = Id;
        using callback_t = std::function<Callback>;
        using result_t = typename callback_t::result_type;

      private:
        std::mutex m_mutex;
        std::atomic_uint64_t m_counter{0};
        std::map<std::uint64_t, callback_t> m_callbacks;

      public:
        void clear();
        void remove(std::uint64_t);
        std::uint64_t add(callback_t &&);

      public:
        template <typename... Params> auto fire(Params &&...);
        template <typename... Params> auto fire(Params &&...) const;
    };
} // namespace ereignis

#include "event.inl"