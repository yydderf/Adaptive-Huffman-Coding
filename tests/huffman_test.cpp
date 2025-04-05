#include <boost/dynamic_bitset.hpp>
#include <spdlog/spdlog.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <gtest/gtest.h>

#include "../src/huffman.h"
#include "../src/dataloader.h"

TEST(HuffmanEncodeDecode, BasicAssertions)
{
    // spdlog::set_level(spdlog::level::debug);
    // data loader
    DataLoader edl("../res/mock_data", 8, MODE_ENC);
    ASSERT_EQ(edl.ok(), true);
    // coder
    Encoder encoder(&edl, 8, "mock_data.compressed.h");
    encoder.proc<uint8_t>();

    DataLoader ddl("mock_data.compressed.h", 8, MODE_DEC);
    ASSERT_EQ(ddl.ok(), true);

    Decoder decoder(&ddl, 8, "mock_data.original.h");
    decoder.proc();
}

TEST(Huffman32EncodeDecode, BasicAssertions)
{
    // spdlog::set_level(spdlog::level::debug);
    // data loader
    DataLoader edl("../res/mock_data", 32, MODE_ENC);
    ASSERT_EQ(edl.ok(), true);
    // coder
    Encoder encoder(&edl, 32, "mock_data.compressed.h32");
    encoder.proc<uint32_t>();

    DataLoader ddl("mock_data.compressed.h32", 32, MODE_DEC);
    ASSERT_EQ(ddl.ok(), true);

    Decoder decoder(&ddl, 32, "mock_data.original.h32");
    decoder.proc<uint32_t>();
}
