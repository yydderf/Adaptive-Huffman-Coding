#include <gtest/gtest.h>

#include "../src/node.h"

TEST(NodeInitTest, BasicAssertions)
{
    uint64_t id;
    uint32_t symbol, weight;
    Node *parent = nullptr;

    Node node_0;
    node_0._info(&id, &symbol, &weight, &parent);
    ASSERT_EQ(id, 0);
    ASSERT_EQ(symbol, NYT);
    ASSERT_EQ(weight, 0);
    ASSERT_EQ(parent, nullptr);

    Node node_1(16);
    node_1._info(&id, &symbol, &weight, &parent);
    ASSERT_EQ(id, 16);
    ASSERT_EQ(symbol, NYT);
    ASSERT_EQ(weight, 0);
    ASSERT_EQ(parent, nullptr);

    Node node_2(17, 71);
    node_2._info(&id, &symbol, &weight, &parent);
    ASSERT_EQ(id, 17);
    ASSERT_EQ(symbol, 71);
    ASSERT_EQ(weight, 0);
    ASSERT_EQ(parent, nullptr);

    Node node_3(18, 81, &node_0);
    node_3._info(&id, &symbol, &weight, &parent);
    ASSERT_EQ(id, 18);
    ASSERT_EQ(symbol, 81);
    ASSERT_EQ(weight, 0);
    ASSERT_EQ(parent, &node_0);
}
