#include <boost/ut.hpp>

#include <ereignis/manager/manager.hpp>

int main()
{
    auto event = ereignis::event<0, void(int)>{};

    return boost::ut::cfg<boost::ut::override>.run();
}
