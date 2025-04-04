#include <spdlog/spdlog.h>

#include "adaptive_huffman.h"
#include "dataloader.h"
#include "argparse.h"

int main(int argc, char **argv)
{
    auto args = Arguments(argc, argv);

    spdlog::info("adaptive huffman init");
    // data loader
    int mode_dec = args.decode ? MODE_DEC : MODE_ENC;
    DataLoader dl(args.ifname, args.bits, READ_ONCE | ADAPTIVE | mode_dec);
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

