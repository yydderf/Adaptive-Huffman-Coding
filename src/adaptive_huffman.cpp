#include <iostream>
#include <spdlog/spdlog.h>

#include "argparse.h"
#include "adaptive_huffman.h"

int main(int argc, char **argv)
{
    auto args = Arguments(argc, argv);

    spdlog::info("adaptive huffman init");

    return 0;
}

