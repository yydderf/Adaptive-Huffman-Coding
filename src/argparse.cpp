#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>

#include <spdlog/spdlog.h>

#include "argparse.h"

void usage(const char *name)
{
    std::cerr << "Usage: " << name << " [-i INPUT] ([-o] [-v {w,i,d}] [-d])" << std::endl
        << std::endl
        << "Adaptive Huffman Encoder / Decoder" << std::endl
        << "options:" << std::endl
        << "  -i\tinput file" << std::endl
        << "  -o\toutput file" << std::endl
        << "  -v\tverbosity {i(default): info, w: warn, d: debug}" << std::endl
        << "  -d\tdecoding mode" << std::endl;
    exit(EXIT_FAILURE);
}

Arguments::Arguments(int argc, char **argv)
{
    int opt;
    while ((opt = getopt(argc, argv, "i:o:b:dv:")) != -1) {
        switch (opt) {
            case 'i':
                this->ifname = optarg;
                break;
            case 'o':
                this->ofname = optarg;
                break;
            case 'v':
                switch (optarg[0]) {
                    case 'w':
                        spdlog::set_level(spdlog::level::warn);
                        break;
                    case 'd':
                        spdlog::set_level(spdlog::level::debug);
                        break;
                    case 'i':
                    default:
                        spdlog::set_level(spdlog::level::info);
                }
                break;
            case 'b':
                this->bits = std::atoi(optarg);
                break;
            case 'd':
                // toggle for decode mode
                this->decode = true;
                break;
            default:
                usage(argv[0]);
        }
    }
    if (ifname == NULL) {
        std::cerr << argv[0] << ": requires an input file" << std::endl;
        usage(argv[0]);
    }

    spdlog::debug("read from: {}", this->ifname);
    spdlog::debug("output to: {}", this->ofname);
}
