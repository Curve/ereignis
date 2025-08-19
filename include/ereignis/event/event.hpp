#pragma once

#include <mutex>
#include <atomic>

#include <map>
#include <memory>
#include <functional>

#include <coco/promise/promise.hpp>
#include <coco/generator/generator.hpp>

namespace ereignis
{
    namespace impl
    {
        template <typename T>
        struct is_event;

        template <typename... Ts>
        struct await_result;

        template <typename Result, typename... Ts>
        struct await_return;
    }; // namespace impl

    template <typename Result, typename... Ts>
    concept AwaitReturn = impl::await_return<Result, Ts...>::value;

    template <auto Id, typename Signature>
    struct event;

    template <typename T, bool Base = false>
    struct listener;

    template <auto Id, typename R, typename... Ts>
    struct event<Id, R(Ts...)>
    {
        static constexpr auto id = Id;

      private:
        using await_result   = impl::await_result<Ts...>::type;
        using clear_callback = std::move_only_function<void()>;

      public:
        using listener  = ereignis::listener<R(Ts...)>;
        using future    = coco::future<await_result>;
        using arguments = std::tuple<Ts...>;
        using result    = R;

      private:
        std::mutex m_clear_mutex;
        clear_callback m_clear_callback;

      private:
        std::mutex m_mutex;
        std::atomic_size_t m_counter{0};
        std::map<std::size_t, std::shared_ptr<listener>> m_listeners;

      private:
        auto copy();
        void cleared();

      public:
        event()                  = default;
        event(const event &)     = delete;
        event(event &&) noexcept = delete;

      public:
        std::size_t add(listener);
        void update(std::size_t, listener);

      public:
        void once(listener::callback);

      public:
        template <typename... Rs>
        auto await(Rs &&...) -> future
            requires AwaitReturn<result, Rs...>;

      public:
        void on_clear(clear_callback);

      public:
        void clear();
        void remove(std::size_t);

      public:
        [[nodiscard]] bool empty();

      public:
        void fire(Ts...)
            requires(std::is_void_v<result>);

        auto fire(Ts...) -> coco::generator<result>
            requires(not std::is_void_v<result>);
    };

    template <typename T>
    concept Event = impl::is_event<T>::value;
} // namespace ereignis

#include "event.inl"
