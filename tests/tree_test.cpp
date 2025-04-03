#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#include "../src/node.h"
#include "../src/tree.h"
#include "../src/dataloader.h"

TEST(TreeInitTest, BasicAssertions)
{
    uint32_t symbol_size = 0;
    Node *node = nullptr;

    Tree tree_0;
    tree_0._info(&symbol_size, &node);
    ASSERT_EQ(symbol_size, 256);

    Tree tree_1(400);
    tree_1._info(&symbol_size, &node);
    ASSERT_EQ(symbol_size, 400);
}

TEST(TreeUpdateTest, BasicAssertions)
{
    Tree tree(26);
    const char *fname = "../res/mock_data";
    DataLoader dataloader(fname, 8, READ_ONCE | MODE_ENC);
    ASSERT_EQ(dataloader.ok(), true);
    std::streamsize read_bytes;
    std::vector<char> *data = dataloader.get(32, &read_bytes);
    for (int i = 0; i < read_bytes; ++i) {
        tree.update(static_cast<uint32_t>((*data)[i]));
        tree.display();
        spdlog::info("------------------------------------------");
    }
}
