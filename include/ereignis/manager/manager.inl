#pragma once

#include "manager.hpp"
#include "../utils/utils.hpp"

namespace ereignis
{
    template <Event... Events>
    template <typename T, typename Visitor>
    constexpr auto manager<Events...>::visit(T id, const Visitor &visitor)
    {
        auto visit = [&]<typename Event>(Event &event)
        {
            if (utils::equals(Event::id, id))
            {
                visitor(event);
            }
        };

        auto unpack = [&](auto &...events)
        {
            (visit(events), ...);
        };

        std::apply(unpack, m_events);
    }

    template <Event... Events>
    template <auto Id>
    consteval auto manager<Events...>::find()
    {
        using tuple = std::tuple<Events...>;

        auto unpack = []<auto I>(this auto &unpack, utils::constant<I>)
        {
            if constexpr (I < std::tuple_size_v<tuple>)
            {
                using current = std::tuple_element_t<I, tuple>;

                if constexpr (utils::equals(current::id, Id))
                {
                    return std::type_identity<current>{};
                }
                else
                {
                    return unpack(utils::constant<I + 1>{});
                }
            }
        };

        return unpack(utils::constant<0>{});
    }

    template <Event... Events>
    template <auto Id>
    auto &manager<Events...>::get()
    {
        return std::get<event<Id>>(m_events);
    }

    template <Event... Events>
    void manager<Events...>::clear(bool force)
    {
        auto visitor = [force](auto &event)
        {
            event.clear(force);
        };

        auto unpack = [&](auto &...events)
        {
            (visitor(events), ...);
        };

        std::apply(unpack, m_events);
    }

    template <Event... Events>
    template <typename T>
    void manager<Events...>::clear(T event, bool force)
    {
        visit(event, [force](auto &event) { event.clear(force); });
    }

    template <Event... Events>
    template <typename T>
    void manager<Events...>::remove(T event, std::size_t id)
    {
        visit(event, [id](auto &event) { event.remove(id); });
    }
} // namespace ereignis
