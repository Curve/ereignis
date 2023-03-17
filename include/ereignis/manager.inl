#pragma once
#include "manager.hpp"

#include <variant>
#include <concepts>
#include <type_traits>

namespace ereignis
{
    template <Event... events> template <auto Id, std::size_t I> constexpr auto &manager<events...>::at()
    {
        using tuple_t = std::tuple<events...>;
        constexpr auto size = std::tuple_size_v<tuple_t>;

        if constexpr (I < size)
        {
            using event = std::tuple_element_t<I, tuple_t>;
            constexpr auto id = event::id;

            if constexpr (std::equality_comparable_with<decltype(Id), decltype(id)> && id == Id)
            {
                return std::get<I>(m_events);
            }
            else
            {
                return at<Id, I + 1>();
            }
        }
    }

    template <Event... events> template <typename T> void manager<events...>::clear(T event)
    {
        std::apply(
            [&](auto &...items) {
                (([&]<auto Id, callback Callback>(ereignis::event<Id, Callback> &item) {
                     if constexpr (std::equality_comparable_with<decltype(Id), T>)
                     {
                         if (Id == event)
                         {
                             item.clear();
                         }
                     }
                 })(items),
                 ...);
            },
            m_events);
    }

    template <Event... events> template <typename T> void manager<events...>::remove(T event, std::size_t id)
    {
        std::apply(
            [&](auto &...items) {
                (([&]<auto Id, typename Callback>(ereignis::event<Id, Callback> &item) {
                     if constexpr (std::equality_comparable_with<decltype(Id), T>)
                     {
                         if (Id == event)
                         {
                             item.remove(id);
                         }
                     }
                 })(items),
                 ...);
            },
            m_events);
    }

    template <Event... events> template <auto Id, std::size_t I> consteval auto manager<events...>::type()
    {
        return []<auto EventId, typename Callback>(event<EventId, Callback> &) { //
            return std::type_identity<Callback>{};
        }(manager<events...>{}.at<Id>());
    }
} // namespace ereignis