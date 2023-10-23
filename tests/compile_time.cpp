#include <boost/ut.hpp>
#include <ereignis/manager.hpp>

#include <concepts>

using namespace boost::ut;
using namespace boost::ut::bdd;

// NOLINTNEXTLINE
suite<"constexpr"> constexpr_suite = []()
{
    using ereignis::event;

    enum class enum_ids
    {
        one,
        two,
    };

    using event_manager = ereignis::manager< //
        event<0, void()>,                    //
        event<1, int()>                      //
        >;

    using event_manager_with_enums = ereignis::manager< //
        event<enum_ids::one, void()>,                   //
        event<enum_ids::two, int()>                     //
        >;

    event_manager dummy{};

    using invoker = decltype(dummy.at<1>().fire());
    using iterator = decltype(dummy.at<1>().fire().begin());

    static_assert(std::ranges::view<invoker>);
    static_assert(std::forward_iterator<iterator>);

    static_assert(std::same_as<typename event_manager::type_t<0>, std::function<void()>>);

    static_assert(std::same_as<typename event_manager::type_t<0>::result_type, void>);
    static_assert(std::same_as<typename event_manager::type_t<1>::result_type, int>);

    static_assert(std::same_as<typename event_manager_with_enums::type_t<enum_ids::one>::result_type, void>);
    static_assert(std::same_as<typename event_manager_with_enums::type_t<enum_ids::two>::result_type, int>);
};
