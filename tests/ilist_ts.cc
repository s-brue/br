#include <gtest/gtest.h>

#include <algorithm>
#include <random>

#include "ilist.h"

class IlistTest: public testing::Test {
protected:
    IlistTest()           = default;
    ~IlistTest() override = default;

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    struct K;
    using LW = br::ilist<K, std::mutex>;

    struct K final: LW::node {
        int a{1};
    };
};


TEST_F(IlistTest, Basic)
{
    LW l;

    l.push_front(new K);
    EXPECT_EQ(l.front(), l.back());
    EXPECT_EQ(l.size(), 1);

    delete l.front();
    EXPECT_TRUE(l.empty());
}

TEST_F(IlistTest, Push)
{
    LW l;

    l.push_back(new K);
    auto* p = l.pop_front();
    EXPECT_NE(p, nullptr);
    EXPECT_EQ(l.size(), 0);
    delete p;

    l.push_front(new K);
    auto* f = l.front();
    EXPECT_NE(f, nullptr);
    delete f;
    EXPECT_EQ(l.size(), 0);

    l.push_front(new K);
    auto* g = l.back();
    EXPECT_NE(g, nullptr);
    delete g;
    EXPECT_EQ(l.size(), 0);
}

TEST_F(IlistTest, Linking)
{
    LW l;

    l.push_front(new K);
    auto* h = l.back();
    auto* j = l.front();
    EXPECT_EQ(h, j);

    h->link_node_after(new K);
    h->link_node_before(new K);

    EXPECT_EQ(l.size(), 3);

    while (const auto* node = l.pop_front()) {
        delete node;
    }

    EXPECT_EQ(l.size(), 0);
    EXPECT_TRUE(l.empty());
}

TEST_F(IlistTest, Updating)
{
    LW l;

    unsigned n = 100;
    while (n--) {
        l.push_back(new K);
    }
    EXPECT_EQ(l.size(), 100);

    unsigned total = 0;
    std::for_each(l.begin(), l.end(), [&](const K& k) { total += k.a; });
    EXPECT_EQ(total, 100);

    std::vector<K*> v;
    for (auto& c : l) {
        v.push_back(&c);
    }

    std::ranges::shuffle(v, std::mt19937(std::random_device{}()));

    for (auto k : v) {
        delete k;
    }

    EXPECT_EQ(l.size(), 0);
}


TEST_F(IlistTest, Clear)
{
    LW l;

    std::vector<K*> v;
    for (unsigned i = 0; i < 100; ++i) {
        l.push_back(new K);
        v.push_back(l.back());
    }

    l.clear();
    EXPECT_TRUE(l.empty());

    for (const auto* k : v) {
        delete k;
    }
}
