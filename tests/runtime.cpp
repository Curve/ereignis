#include <vector>
#include <catch2/catch.hpp>
#include <ereignis/manager.hpp>

TEST_CASE("Perform runtime checks", "[runtime]")
{
    using ereignis::event;

    ereignis::manager<          //
        event<0, void()>,       //
        event<1, int()>,        //
        event<2, bool(int)>,    //
        event<3, void(bool &)>, //
        event<4, void(bool *)>  //
        >
        event_manager;

    std::size_t first_call_count{0};

    event_manager.at<0>().add([&] { first_call_count++; });

    auto id = event_manager.at<0>().add([&] { first_call_count++; });
    auto id2 = event_manager.at<0>().add([&] { first_call_count++; });

    event_manager.at<0>().fire();
    REQUIRE(first_call_count == 3);

    event_manager.at<0>().remove(id);
    event_manager.at<0>().fire();

    REQUIRE(first_call_count == 5);

    event_manager.remove(0, id2);
    event_manager.at<0>().fire();

    REQUIRE(first_call_count == 6);

    event_manager.at<0>().clear();
    event_manager.at<0>().fire();

    REQUIRE(first_call_count == 6);

    event_manager.at<1>().add([] { return 10; });
    REQUIRE(*event_manager.at<1>().fire().begin() == 10);

    std::vector<int> args{0, 2, 4, 5, 6, 7};
    event_manager.at<2>().add([](int i) { return (i % 1) == 0; });
    event_manager.at<2>().add([](int i) { return (i % 2) == 0; });

    for (const auto &arg : args)
    {
        bool first = true;

        for (const auto &result : event_manager.at<2>().fire(arg))
        {
            if (first)
            {
                REQUIRE(result == ((arg % 1) == 0));
            }
            else
            {
                REQUIRE(result == ((arg % 2) == 0));
            }
            first = false;
        }
    }

    event_manager.at<3>().add([](bool &value) { value = true; });

    bool value = false;
    event_manager.at<3>().fire(value);

    REQUIRE(value);

    event_manager.at<4>().add([](bool *value) { (*value) = false; });
    event_manager.at<4>().fire(&value);

    REQUIRE(!value);

    event_manager.clear(3);
    event_manager.at<3>().fire(value);

    REQUIRE(!value);
}