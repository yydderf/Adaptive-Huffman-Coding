#include <iostream>
#include <spdlog/spdlog.h>

#include "argparse.h"
#include "dataloader.h"
#include "coder.h"
#include "huffman.h"

int main(int argc, char **argv)
{
    Arguments args(argc, argv);
    spdlog::info("huffman init");

    // data loader
    DataLoader dl(args.ifname);
    if (!dl.ok()) {
        return 1;
    }
    // coder
    Coder coder(&dl, args.bits, args.ofname);

    return 0;
}
