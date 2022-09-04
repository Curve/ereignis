#pragma once
#include "manager.hpp"
#include "utils/type_identity.hpp"

namespace ereignis
{
    template <class... Events> template <auto Id, std::size_t I> constexpr auto event_manager<Events...>::callback()
    {
        constexpr auto size = std::tuple_size_v<tuple_t>;
        if constexpr (I < size)
        {
            using event_t = std::tuple_element_t<I, tuple_t>;
            constexpr auto id = event_t::id;

            if constexpr (id == Id)
            {
                return type_identity<typename event_t::callback_t>{};
            }
            else
            {
                return callback<Id, I + 1>();
            }
        }
    }

    template <class... Events> template <auto Id, std::size_t I> constexpr auto &event_manager<Events...>::at()
    {
        constexpr auto size = std::tuple_size_v<tuple_t>;
        if constexpr (I < size)
        {
            using event_t = std::tuple_element_t<I, tuple_t>;
            constexpr auto id = event_t::id;

            if constexpr (id == Id)
            {
                return std::get<I>(m_events);
            }
            else
            {
                return at<Id, I + 1>();
            }
        }
    }
} // namespace ereignis