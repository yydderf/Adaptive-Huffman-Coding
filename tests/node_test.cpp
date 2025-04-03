#include <gtest/gtest.h>

#include "../src/node.h"

TEST(NodeInitTest, BasicAssertions)
{
    uint64_t id;
    uint32_t symbol, weight;
    Node *parent = nullptr;
    bool NYT;

    Node node_0;
    node_0._info(&id, &symbol, &weight, &parent, &NYT);
    ASSERT_EQ(id, 0);
    ASSERT_EQ(weight, 0);
    ASSERT_EQ(parent, nullptr);
    ASSERT_EQ(NYT, true);

    Node node_1(16);
    node_1._info(&id, &symbol, &weight, &parent, &NYT);
    ASSERT_EQ(id, 16);
    ASSERT_EQ(weight, 0);
    ASSERT_EQ(parent, nullptr);
    ASSERT_EQ(NYT, true);

    Node node_2(17, 71);
    node_2._info(&id, &symbol, &weight, &parent, &NYT);
    ASSERT_EQ(id, 17);
    ASSERT_EQ(symbol, 71);
    ASSERT_EQ(weight, 0);
    ASSERT_EQ(parent, nullptr);
    ASSERT_EQ(NYT, false);

    Node node_4(18, 81, 1, &node_0);
    node_4._info(&id, &symbol, &weight, &parent, &NYT);
    ASSERT_EQ(id, 18);
    ASSERT_EQ(symbol, 81);
    ASSERT_EQ(weight, 0);
    ASSERT_EQ(parent, &node_0);
    ASSERT_EQ(NYT, false);
}

TEST(NodeSetTest, BasicAssertions)
{
    uint64_t id;
    uint32_t symbol, weight;
    Node *parent = nullptr;
    bool NYT;

    Node node_0;
    node_0._info(&id, &symbol, &weight, &parent, &NYT);
    ASSERT_EQ(id, 0);
    ASSERT_EQ(weight, 0);
    ASSERT_EQ(parent, nullptr);
    ASSERT_EQ(NYT, true);

    uint32_t i_symbol = 10;
    uint32_t o_symbol;
    node_0.set(i_symbol);
    node_0._info(&id, &o_symbol, &weight, &parent, &NYT);
    ASSERT_EQ(o_symbol, 10);
    ASSERT_EQ(NYT, false);
}
