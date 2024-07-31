#pragma once

#include <map>
#include <tuple>
#include <ranges>

namespace ereignis
{
    namespace impl
    {
        template <typename T, typename... As>
        concept can_apply = requires(T callback, std::tuple<As...> tuple) {
            { std::apply(callback, tuple) };
        };
    } // namespace impl

    template <typename T, typename... As>
        requires impl::can_apply<T, As...>
    class invoker
    {
        class iterator;

      private:
        using args_t      = std::tuple<As...>;
        using callbacks_t = std::map<std::size_t, T>;

      private:
        args_t m_args;
        callbacks_t m_callbacks;

      public:
        invoker(callbacks_t callbacks, args_t args);

      public:
        auto end();
        auto begin();
    };

} // namespace ereignis

template <typename T, typename... As>
    requires ereignis::impl::can_apply<T, As...>
constexpr inline bool std::ranges::enable_view<ereignis::invoker<T, As...>> = true;

#include "invoker.inl"
