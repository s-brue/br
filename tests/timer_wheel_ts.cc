#include <gtest/gtest.h>

#include "timer_wheel.h"

class TimerWheelTest: public testing::Test {
protected:
    TimerWheelTest()           = default;
    ~TimerWheelTest() override = default;

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    struct K final: br::ts_expirable{
        int a{0};

        void expire() override
        {
            std::cout << "Expiring!" << std::endl;
            a = 1337;
        }
    };

    struct L final: br::expirable {
        int a{0};

        void expire() override
        {
            std::cout << "Expiring!" << std::endl;
            a = 1337;
        }
    };

};


TEST_F(TimerWheelTest, Basic)
{
    const std::chrono::time_point<std::chrono::steady_clock> t;

    br::ts_timer_wheel tw(std::chrono::seconds(1), 100, t);

    K k;
    K l;

    tw.publish(&k, t+std::chrono::seconds(200));
    tw.publish(&l, t+std::chrono::seconds(100));


    tw.check_expiration(t+std::chrono::seconds(99));

    EXPECT_EQ(k.a, 0);
    EXPECT_EQ(l.a, 0);

    tw.check_expiration(t+std::chrono::seconds(101));

    EXPECT_EQ(k.a, 0);
    EXPECT_EQ(l.a, 1337);

    tw.check_expiration(t+std::chrono::seconds(199));

    EXPECT_EQ(k.a, 0);
    EXPECT_EQ(l.a, 1337);

    tw.check_expiration(t+std::chrono::seconds(200));

    EXPECT_EQ(k.a, 0);
    EXPECT_EQ(l.a, 1337);

    tw.check_expiration(t+std::chrono::seconds(201));

    EXPECT_EQ(k.a, 1337);
    EXPECT_EQ(l.a, 1337);
}
