#pragma once

#include "manager.hpp"

#include <concepts>
#include <type_traits>

namespace ereignis
{
    template <detail::ereignis_event... events>
    template <typename T, typename Visitor>
    void manager<events...>::find(T id, Visitor visitor)
    {
        auto fn = [&]<auto Id, detail::callback Callback>(ereignis::event<Id, Callback> &item)
        {
            if constexpr (std::equality_comparable_with<decltype(Id), T>)
            {
                if (Id == id)
                {
                    visitor(item);
                }
            }
        };

        std::apply([&]<typename... Ts>(Ts &&...items) { (fn(std::forward<Ts>(items)), ...); }, m_events);
    }

    template <detail::ereignis_event... events>
    template <auto Id, std::size_t I>
    constexpr auto &manager<events...>::at()
    {
        using tuple_t       = std::tuple<events...>;
        constexpr auto size = std::tuple_size_v<tuple_t>;

        if constexpr (I < size)
        {
            using event       = std::tuple_element_t<I, tuple_t>;
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

    template <detail::ereignis_event... events>
    template <typename T>
    void manager<events...>::clear(T event)
    {
        find(event, [](auto &&item) { item.clear(); });
    }

    template <detail::ereignis_event... events>
    template <typename T>
    void manager<events...>::remove(T event, std::size_t id)
    {
        find(event, [id](auto &&item) { item.remove(id); });
    }

    template <detail::ereignis_event... events>
    template <auto Id, std::size_t I>
    consteval auto manager<events...>::type()
    {
        return []<auto EventId, detail::callback Callback>(event<EventId, Callback> &) { //
            return std::type_identity<std::function<Callback>>{};
        }(manager<events...>{}.at<Id>());
    }
} // namespace ereignis
