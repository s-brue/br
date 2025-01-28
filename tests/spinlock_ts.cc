#include <gtest/gtest.h>

#include "spinlock.h"

class SpinLockTest: public testing::Test {
protected:
    SpinLockTest()           = default;
    ~SpinLockTest() override = default;

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

};


TEST_F(SpinLockTest, Basic)
{
    br::spinlock sl;

    sl.lock();


    std::jthread t {[&sl](){std::unique_lock<br::spinlock> l(sl); printf("done\n");} };

    printf("Hello\n");

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    sl.unlock();


}
