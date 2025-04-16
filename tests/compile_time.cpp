#include <boost/ut.hpp>
#include <ereignis/manager/manager.hpp>

#include <cstdint>
#include <concepts>

using namespace boost::ut;
using namespace boost::ut::bdd;

enum class enum_ids : std::uint8_t
{
    one,
    two,
};

// NOLINTNEXTLINE
suite<"constexpr"> constexpr_suite = []()
{
    using ereignis::event;

    using event_manager = ereignis::manager< //
        event<0, void()>,                    //
        event<1, int()>                      //
        >;

    using another_manager = ereignis::manager< //
        event<enum_ids::one, void()>,          //
        event<enum_ids::two, int()>            //
        >;

    event_manager dummy{};

    using invoker = decltype(dummy.get<1>().fire());
    using until   = decltype(dummy.get<1>().fire().find(1));

    expect(std::ranges::view<invoker>);
    expect(std::same_as<until, std::optional<int>>);

    expect(std::same_as<event_manager::event<0>::callback, std::move_only_function<void()>>);

    expect(std::same_as<event_manager::event<0>::result, void>);
    expect(std::same_as<event_manager::event<1>::result, int>);

    expect(std::same_as<another_manager::event<enum_ids::one>::result, void>);
    expect(std::same_as<another_manager::event<enum_ids::two>::result, int>);
};
