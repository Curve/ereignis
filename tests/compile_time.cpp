#define CONFIG_CATCH_MAIN
#include <catch2/catch.hpp>
#include <ereignis/manager.hpp>

TEST_CASE("Perform compile time checks", "[constexpr]")
{
    using ereignis::event;

    enum class enum_ids
    {
        one,
        two,
    };

    using event_manager = ereignis::event_manager< //
        event<0, void()>,                          //
        event<1, int()>                            //
        >;

    using event_manager_with_enums = ereignis::event_manager< //
        event<enum_ids::one, void()>,                         //
        event<enum_ids::two, int()>                           //
        >;

    static_assert(std::is_same_v<event_manager::callback_t<0>::result_type, void>);
    static_assert(std::is_same_v<event_manager::callback_t<1>::result_type, int>);

    static_assert(std::is_same_v<event_manager_with_enums::callback_t<enum_ids::one>::result_type, void>);
    static_assert(std::is_same_v<event_manager_with_enums::callback_t<enum_ids::two>::result_type, int>);
}