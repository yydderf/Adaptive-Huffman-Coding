#ifndef _ARGPARSE_H
#define _ARGPARSE_H

#include <fstream>

void usage(const char *name);

class Arguments {
public:
    Arguments(int argc, char **argv);
    std::ifstream ifs;
    std::ofstream ofs;
private:
    bool verbose = false;
    const char *ifname = NULL;
    const char *ofname = "out";
};

#endif
