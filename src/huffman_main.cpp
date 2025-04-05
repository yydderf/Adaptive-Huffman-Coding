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
    int mode_dec = args.decode ? MODE_DEC : MODE_ENC;
    DataLoader dl(args.ifname, args.bits, READ_ONCE | mode_dec);
    if (!dl.ok()) {
        return 1;
    }
    // coder
    if (args.decode) {
        Decoder decoder(&dl, args.bits, args.ofname);
        decoder.proc();
    } else {
        Encoder encoder(&dl, args.bits, args.ofname);
        encoder.proc();
    }

    return 0;
}

