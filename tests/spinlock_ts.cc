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
    br::SpinLock sl;

    sl.lock();


    std::jthread t {[&sl](){std::unique_lock<br::SpinLock> l(sl); printf("done\n");} };

    printf("Hello\n");

    std::this_thread::sleep_for(std::chrono::seconds(3));

    sl.unlock();


}
