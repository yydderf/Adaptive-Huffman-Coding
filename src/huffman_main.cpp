#include <iostream>
#include <spdlog/spdlog.h>

#include "argparse.h"
#include "dataloader.h"
#include "coder.h"
#include "huffman.h"

template <typename F>
void dispatch_bits(int bits, F f) {
    spdlog::info("use {}-bit", bits);
    switch (bits) {
        case 8: f(uint8_t{}); break;
        case 32: f(uint32_t{}); break;
        default: spdlog::error("Unsupported bit width");
    }
}

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
    // if (args.decode) {
    //     Decoder decoder(&dl, args.bits, args.ofname);
    //     decoder.proc<_type>();
    // } else {
    //     Encoder encoder(&dl, args.bits, args.ofname);
    //     encoder.proc<_type>();
    // }

    // switch datatype at runtime
    dispatch_bits(args.bits, [&](auto type_tag) {
        using _type = decltype(type_tag);
        if (args.decode) {
            Decoder decoder(&dl, args.bits, args.ofname);
            decoder.proc<_type>();
        } else {
            Encoder encoder(&dl, args.bits, args.ofname);
            encoder.proc<_type>();
        }
    });


    return 0;
}

