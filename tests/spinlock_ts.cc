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
}
