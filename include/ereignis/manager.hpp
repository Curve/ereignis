#pragma once
#include "event.hpp"
#include "utils/type_traits.hpp"

#include <tuple>

namespace ereignis
{
    template <class... Events> class event_manager
    {
        using tuple_t = std::tuple<Events...>;
        static_assert((is_event_v<Events> && ...));

      private:
        tuple_t m_events;

      public:
        template <auto Id, std::size_t I = 0> static constexpr auto callback();
        template <auto Id> using callback_t = typename decltype(callback<Id>())::type;

      public:
        template <auto Id, std::size_t I = 0> constexpr auto &at();

      public:
        template <typename EventId> void clear(EventId event);
        template <typename EventId> void remove(EventId event, std::uint64_t id);
    };
} // namespace ereignis

#include "manager.inl"