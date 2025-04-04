#include <spdlog/spdlog.h>

#include "adaptive_huffman.h"
#include "dataloader.h"
#include "argparse.h"

int main(int argc, char **argv)
{
    auto args = Arguments(argc, argv);

    spdlog::info("adaptive huffman init");
    // data loader
    DataLoader dl(args.ifname, args.bits, READ_ONCE | MODE_ENC);
    if (!dl.ok()) {
        return 1;
    }
    // coder
    Encoder encoder(&dl, args.bits, args.ofname);
    encoder.proc();

    return 0;
}

