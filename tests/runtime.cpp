#include <boost/ut.hpp>
#include <ereignis/manager.hpp>

using namespace boost::ut;
using namespace boost::ut::bdd;

// NOLINTNEXTLINE
suite<"runtime"> runtime_suite = []()
{
    using ereignis::event;

    ereignis::manager<          //
        event<0, void()>,       //
        event<1, int()>,        //
        event<2, bool(int)>,    //
        event<3, void(bool &)>, //
        event<4, void(bool *)>, //
        event<5, void(bool &)>  //
        >
        event_manager;

    std::size_t first_call_count{0};

    event_manager.at<0>().add([&] { first_call_count++; });

    auto id = event_manager.at<0>().add([&] { first_call_count++; });
    auto id2 = event_manager.at<0>().add([&] { first_call_count++; });

    event_manager.at<0>().fire();
    expect(eq(first_call_count, 3));

    event_manager.at<0>().remove(id);
    event_manager.at<0>().fire();

    expect(eq(first_call_count, 5));

    event_manager.remove(0, id2);
    event_manager.at<0>().fire();

    expect(eq(first_call_count, 6));

    event_manager.at<0>().clear();
    event_manager.at<0>().fire();

    expect(eq(first_call_count, 6));

    event_manager.at<1>().add([] { return 10; });
    expect(eq(*event_manager.at<1>().fire().begin(), 10));

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
                expect(result == ((arg % 1) == 0));
            }
            else
            {
                expect(result == ((arg % 2) == 0));
            }

            first = false;
        }
    }

    event_manager.at<3>().add([](bool &value) { value = true; });

    bool value = false;
    event_manager.at<3>().fire(value);

    expect(value);

    event_manager.at<4>().add([](bool *value) { (*value) = false; });
    event_manager.at<4>().fire(&value);

    expect(!value);

    event_manager.clear(3);
    event_manager.at<3>().fire(value);

    expect(!value);

    event_manager.at<5>().once([](bool &value) { value = !value; });

    bool once = false;

    event_manager.at<5>().fire(once);
    event_manager.at<5>().fire(once);

    expect(once);
};
