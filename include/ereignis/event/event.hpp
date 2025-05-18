#pragma once

#include <mutex>
#include <memory>

#include <map>
#include <functional>

#include <coco/generator/generator.hpp>

namespace ereignis
{
    namespace impl
    {
        template <typename T>
        struct is_event;
    };

    template <auto Id, typename Signature>
    struct event
    {
        static constexpr auto id = Id;

      public:
        using callback = std::move_only_function<Signature>;
        using result   = callback::result_type;

      private:
        using clear_callback = std::move_only_function<void()>;

      private:
        std::mutex m_mutex;
        std::mutex m_clear_mutex;

      private:
        std::size_t m_counter;
        clear_callback m_clear_callback;
        std::map<std::size_t, std::shared_ptr<callback>> m_callbacks;

      private:
        auto copy();

      public:
        void once(callback);
        std::size_t add(callback);

      public:
        void on_clear(clear_callback);

      public:
        void clear();
        void remove(std::size_t);

      public:
        [[nodiscard]] bool empty();

      public:
        template <typename... Ts>
        void fire(Ts &&...)
            requires(std::is_void_v<result>);

        template <typename... Ts>
        auto fire(Ts &&...) -> coco::generator<result>
            requires(not std::is_void_v<result>);
    };

    template <typename T>
    concept Event = impl::is_event<T>::value;
} // namespace ereignis

#include "event.inl"
