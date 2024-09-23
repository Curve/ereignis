#pragma once

#include "manager.hpp"

#include <concepts>
#include <type_traits>

namespace ereignis
{
    namespace impl
    {
        template <auto T>
        using constant = std::integral_constant<decltype(T), T>;

        template <auto Id, event... events>
        consteval auto type()
        {
            using event = std::decay_t<decltype(manager<events...>{}.template at<Id>())>;
            return std::type_identity<typename event::callback>{};
        }
    } // namespace impl

    template <impl::event... events>
    template <typename T, typename Visitor>
    constexpr void manager<events...>::find(T id, Visitor &&visitor)
    {
        auto fn = [id, visitor = std::forward<Visitor>(visitor)]<auto Id, typename Callback>(
                      ereignis::event<Id, Callback> &event)
        {
            if constexpr (std::equality_comparable_with<T, decltype(Id)>)
            {
                if (Id != id)
                {
                    return;
                }

                visitor(event);
            }
        };

        std::apply([&fn]<typename... Ts>(Ts &...items) { (fn(items), ...); }, m_events);
    }

    template <impl::event... events>
    template <auto Id>
    constexpr auto &manager<events...>::at()
    {
        auto at_impl = [this]<auto I>(auto &self, impl::constant<I>) -> auto &
        {
            if constexpr (I < sizeof...(events))
            {
                using event       = std::tuple_element_t<I, std::tuple<events...>>;
                constexpr auto id = event::id;

                if constexpr (std::equality_comparable_with<decltype(Id), decltype(id)> && id == Id)
                {
                    return std::get<I>(m_events);
                }
                else
                {
                    return self(self, impl::constant<I + 1>{});
                }
            }
        };

        return at_impl(at_impl, impl::constant<0>{});
    }

    template <impl::event... events>
    template <typename T>
    void manager<events...>::clear(T event)
    {
        find(event, [](auto &item) { item.clear(); });
    }

    template <impl::event... events>
    template <typename T>
    void manager<events...>::remove(T event, std::size_t id)
    {
        find(event, [id](auto &item) { item.remove(id); });
    }
} // namespace ereignis
