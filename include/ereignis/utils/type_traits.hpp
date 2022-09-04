#pragma once

namespace ereignis
{
    template <auto, typename> struct event;

    template <class Type> struct is_event
    {
        static constexpr bool value = false;
    };

    template <auto Id, typename Callback> struct is_event<event<Id, Callback>>
    {
        static constexpr bool value = true;
    };

    template <class Type> inline constexpr bool is_event_v = is_event<Type>::value;

    template <typename Type> struct remove_const_ref
    {
        using type = Type;
    };

    template <typename Type> struct remove_const_ref<const Type &>
    {
        using type = Type;
    };

    template <class Type> using remove_const_ref_t = typename remove_const_ref<Type>::type;
} // namespace ereignis