#pragma once

#include "event.hpp"

#include <tuple>

namespace ereignis
{
    template <typename T>
    concept ereignis_event = requires(T t) {
        []<auto Id, callback Callback>(event<Id, Callback> &) {
        }(t);
    };

    template <ereignis_event... events>
    class manager
    {
        std::tuple<events...> m_events;

      private:
        template <typename T, typename Visitor>
        void find(T, Visitor);

      public:
        template <auto Id, std::size_t I = 0>
        constexpr auto &at();

      public:
        template <typename T>
        void clear(T event);

        template <typename T>
        void remove(T event, std::size_t id);

      public:
        template <auto Id, std::size_t I = 0>
        static consteval auto type();

        template <auto Id>
        using type_t = typename decltype(type<Id>())::type;
    };
} // namespace ereignis

#include "manager.inl"
