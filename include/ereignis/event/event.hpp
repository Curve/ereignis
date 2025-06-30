#pragma once

#include <mutex>
#include <memory>

#include <map>
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

    template <auto Id, typename R, typename... Ts>
    struct event<Id, R(Ts...)>
    {
        static constexpr auto id = Id;

      private:
        using await_result   = impl::await_result<Ts...>::type;
        using clear_callback = std::move_only_function<void()>;

      public:
        using callback  = std::move_only_function<R(Ts...)>;
        using future    = coco::future<await_result>;
        using arguments = std::tuple<Ts...>;
        using result    = R;

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
        template <typename... Us>
        void fire(Us &&...)
            requires(std::invocable<callback, Us...> and std::is_void_v<result>);

        template <typename... Us>
        auto fire(Us &&...) -> coco::generator<result>
            requires(std::invocable<callback, Us...> and not std::is_void_v<result>);
    };

    template <typename T>
    concept Event = impl::is_event<T>::value;
} // namespace ereignis

#include "event.inl"
