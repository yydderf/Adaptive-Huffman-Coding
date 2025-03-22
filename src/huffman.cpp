#include <iostream>
#include <spdlog/spdlog.h>

#include "argparse.h"
#include "huffman.h"

int main(int argc, char **argv)
{
    auto args = Arguments(argc, argv);
    spdlog::info("huffman init");

    return 0;
}
