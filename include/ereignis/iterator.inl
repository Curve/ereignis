#pragma once
#include "iterator.hpp"

namespace ereignis
{
    template <typename Type, typename Callback, typename... Params>
    iterator<Type, Callback, Params...>::iterator(std::tuple<Params...> &params, iterator_t iterator) : m_iterator(iterator), m_params(params)
    {
    }

    template <typename Type, typename Callback, typename... Params> Type iterator<Type, Callback, Params...>::operator*()
    {
        return std::apply(m_iterator->second, m_params);
    }

    template <typename Type, typename Callback, typename... Params> Type iterator<Type, Callback, Params...>::operator*() const
    {
        return std::apply(m_iterator->second, m_params);
    }

    template <typename Type, typename Callback, typename... Params> iterator<Type, Callback, Params...> &iterator<Type, Callback, Params...>::operator++()
    {
        m_iterator++;
        return *this;
    }

    template <typename Type, typename Callback, typename... Params> iterator<Type, Callback, Params...> iterator<Type, Callback, Params...>::operator++() const
    {
        return iterator(m_params, ++m_iterator);
    }

    template <typename Type, typename Callback, typename... Params> bool iterator<Type, Callback, Params...>::operator!=(const iterator<Type, Callback, Params...> &other) const
    {
        return other.m_iterator != m_iterator;
    }
} // namespace ereignis