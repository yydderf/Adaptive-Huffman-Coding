#include <gtest/gtest.h>

#include "../src/dataloader.h"

TEST(DataLoaderInitTest, BasicAssertions)
{
    const char *fname = "../res/test_data";
    DataLoader dataloader(fname, 8, READ_ONCE | MODE_ENC);
    ASSERT_EQ(dataloader.ok(), true);
}

TEST(DataLoaderGetTest, BasicAssertions)
{
    const char *fname = "../res/test_data";
    const char *test_data = "aaaaaaaaa\ntest_data\nbbbbbbbbb\n";
    DataLoader dataloader(fname, 8, READ_ONCE | MODE_ENC);
    ASSERT_EQ(dataloader.ok(), true);
    std::streamsize read_bytes;
    std::vector<char> *data = dataloader.get(32, &read_bytes);
    for (int i = 0; i < read_bytes; ++i) {
        ASSERT_EQ(test_data[i], (*data)[i]);
    }
}
