#include <gtest/gtest.h>

#include "arch_info.h"

class ArchInfoTest: public testing::Test {
protected:
    ArchInfoTest()           = default;
    ~ArchInfoTest() override = default;

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

};


TEST_F(ArchInfoTest, Basic)
{
    br::arch_info ai;
    std::cout << "Number of detected NUMA nodes: " << ai.number_of_numa_nodes() << std::endl;

    for (auto nnode : ai.numa_nodes()) {
        std::cout << "Node has " << nnode.cpus().size() << " CPUs: ";
        for (auto cpu : nnode.cpus()) {
            std::cout << "[" << cpu.id() << "] ";
        }
        std::cout << std::endl;
    }
}
