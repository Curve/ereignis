#pragma once

#include "../event/event.hpp"

#include <tuple>

namespace ereignis
{
    template <Event... Events>
    class manager
    {
        std::tuple<Events...> m_events;

      private:
        template <auto Id>
        static consteval auto find();

      private:
        template <typename T, typename Visitor>
        constexpr auto visit(T, const Visitor &);

      public:
        manager()                    = default;
        manager(const manager &)     = delete;
        manager(manager &&) noexcept = delete;

      public:
        template <auto Id>
        using event = decltype(find<Id>())::type;

      public:
        template <auto Id>
        auto &get();

      public:
        template <typename T>
        void clear(T);

        template <typename T>
        void remove(T, std::size_t);
    };
} // namespace ereignis

#include "manager.inl"
