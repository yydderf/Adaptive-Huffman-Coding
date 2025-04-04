#include <boost/dynamic_bitset.hpp>
#include <spdlog/spdlog.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <gtest/gtest.h>

#include "../src/adaptive_huffman.h"
#include "../src/dataloader.h"
#include "../src/utils.h"
#include "../src/node.h"
#include "../src/tree.h"

TEST(AdaptiveHuffmanEncodeDecode, BasicAssertions)
{
    // spdlog::set_level(spdlog::level::debug);
    // data loader
    DataLoader edl("../res/mock_data", 8, MODE_ENC | ADAPTIVE);
    ASSERT_EQ(edl.ok(), true);
    // coder
    Encoder encoder(&edl, 8, "mock_data.compressed");
    encoder.proc();

    DataLoader ddl("mock_data.compressed", 8, MODE_DEC | ADAPTIVE);
    ASSERT_EQ(ddl.ok(), true);

    Decoder decoder(&ddl, 8, "mock_data.original");
    decoder.proc();
}

