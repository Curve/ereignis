#pragma once

#include "invoker.hpp"

#include <iterator>

namespace ereignis
{
    template <typename T, typename... As>
        requires impl::can_apply<T, As...>
    class invoker<T, As...>::iterator
    {
        using iterator_t = callbacks_t::iterator;

      public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = T::result_type;

      private:
        args_t m_args;
        iterator_t m_iterator;

      public:
        iterator() = default;

      public:
        iterator(iterator_t iterator, args_t args) : m_args(std::move(args)), m_iterator(std::move(iterator)) {}

      public:
        value_type operator*()
        {
            return std::apply(m_iterator->second, m_args);
        }

        value_type operator*() const
        {
            return std::apply(m_iterator->second, m_args);
        }

      public:
        iterator &operator++()
        {
            ++m_iterator;
            return *this;
        }

        iterator operator++(int)
        {
            auto copy = *this;
            ++m_iterator;

            return copy;
        }

      public:
        bool operator==(const iterator &other) const
        {
            return m_iterator == other.m_iterator;
        }
    };

    template <typename T, typename... As>
        requires impl::can_apply<T, As...>
    invoker<T, As...>::invoker(callbacks_t callbacks, args_t args)
        : m_args(std ::move(args)), m_callbacks(std::move(callbacks))
    {
    }

    template <typename T, typename... As>
        requires impl::can_apply<T, As...>
    auto invoker<T, As...>::begin()
    {
        return iterator{m_callbacks.begin(), m_args};
    }

    template <typename T, typename... As>
        requires impl::can_apply<T, As...>
    auto invoker<T, As...>::end()
    {
        return iterator{m_callbacks.end(), m_args};
    }
} // namespace ereignis
