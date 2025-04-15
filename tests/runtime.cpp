#include <boost/ut.hpp>
#include <ereignis/manager/manager.hpp>

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

    event_manager.get<0>().add([&] { first_call_count++; });

    auto id  = event_manager.get<0>().add([&] { first_call_count++; });
    auto id2 = event_manager.get<0>().add([&] { first_call_count++; });

    event_manager.get<0>().fire();
    expect(eq(first_call_count, 3ul));

    event_manager.get<0>().remove(id);
    event_manager.get<0>().fire();

    expect(eq(first_call_count, 5ul));

    event_manager.remove(0, id2);
    event_manager.get<0>().fire();

    expect(eq(first_call_count, 6ul));

    event_manager.get<0>().clear();
    event_manager.get<0>().fire();

    expect(eq(first_call_count, 6ul));

    event_manager.get<1>().add([] { return 10; });
    expect(eq(*event_manager.get<1>().fire().begin(), 10));

    std::size_t second_call_count{0};

    event_manager.get<1>().add(
        [&]
        {
            second_call_count++;
            return 0;
        });

    event_manager.get<1>().add(
        [&]
        {
            second_call_count++;
            return 1;
        });

    event_manager.get<1>().add(
        [&]
        {
            second_call_count++;
            return 2;
        });

    auto res_until = event_manager.get<1>().fire().find(1);

    expect(res_until.has_value() && eq(res_until.value(), 1));
    expect(eq(second_call_count, 2ul));

    auto res_during = event_manager.get<1>().fire().skip(0);

    expect(res_during.has_value() && eq(res_during.value(), 10));
    expect(eq(second_call_count, 2ul));

    std::vector<int> args{0, 2, 4, 5, 6, 7};

    event_manager.get<2>().add([](int i) { return (i % 1) == 0; });
    event_manager.get<2>().add([](int i) { return (i % 2) == 0; });

    for (const auto &arg : args)
    {
        bool first = true;

        for (const auto &result : event_manager.get<2>().fire(arg))
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

    event_manager.get<3>().add([](bool &value) { value = true; });

    bool value = false;
    event_manager.get<3>().fire(value);

    expect(value);

    event_manager.get<4>().add([](bool *value) { (*value) = false; });
    event_manager.get<4>().fire(&value);

    expect(!value);

    event_manager.clear(3);
    event_manager.get<3>().fire(value);

    expect(!value);

    event_manager.get<5>().once([](bool &value) { value = !value; });

    bool once = false;

    event_manager.get<5>().fire(once);
    event_manager.get<5>().fire(once);

    expect(once);
};
