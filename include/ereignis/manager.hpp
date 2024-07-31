#pragma once

#include "event.hpp"

#include <tuple>

namespace ereignis
{
    namespace impl
    {
        template <typename T>
        struct is_event : std::false_type
        {
        };

        template <auto Id, typename T>
        struct is_event<event<Id, T>> : std::true_type
        {
        };

        template <typename T>
        concept event = requires() { requires is_event<T>::value; };

        template <auto Id, event... events>
        consteval auto type();
    } // namespace impl

    template <impl::event... events>
    class manager
    {
        std::tuple<events...> m_events;

      private:
        template <typename T, typename Visitor>
        constexpr void find(T, Visitor &&);

      public:
        template <auto Id>
        constexpr auto &at();

      public:
        template <typename T>
        void clear(T event);

        template <typename T>
        void remove(T event, std::size_t id);

      public:
        template <auto Id>
        using type = decltype(impl::type<Id, events...>())::type;
    };
} // namespace ereignis

#include "manager.inl"
