#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>

#include <spdlog/spdlog.h>

#include "argparse.h"

void usage(const char *name)
{
    std::cerr << "Usage: " << name << " [-i input] ([-o output])" << std::endl;
    exit(EXIT_FAILURE);
}

Arguments::Arguments(int argc, char **argv)
{
    int opt;
    while ((opt = getopt(argc, argv, "i:o:v")) != -1) {
        switch (opt) {
            case 'i':
                this->ifname = optarg;
                break;
            case 'o':
                this->ofname = optarg;
                break;
            case 'v':
                this->verbose = true;
                spdlog::set_level(spdlog::level::debug);
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

    this->ifs.open(this->ifname, std::fstream::in);
    this->ofs.open(this->ofname, std::fstream::out);
}
