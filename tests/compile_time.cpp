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

    using another_manager = ereignis::manager< //
        event<enum_ids::one, void()>,          //
        event<enum_ids::two, int()>            //
        >;

    event_manager dummy{};

    using invoker  = decltype(dummy.at<1>().fire());
    using until    = decltype(dummy.at<1>().until(1));
    using iterator = decltype(dummy.at<1>().fire().begin());

    expect(std::ranges::view<invoker>);
    expect(std::forward_iterator<iterator>);
    expect(std::same_as<until, std::optional<int>>);

    expect(std::same_as<event_manager::type<0>, std::function<void()>>);

    expect(std::same_as<event_manager::type<0>::result_type, void>);
    expect(std::same_as<event_manager::type<1>::result_type, int>);

    expect(std::same_as<another_manager::template type<enum_ids::one>::result_type, void>);
    expect(std::same_as<another_manager::template type<enum_ids::two>::result_type, int>);
};
