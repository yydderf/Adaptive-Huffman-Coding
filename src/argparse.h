#ifndef _ARGPARSE_H
#define _ARGPARSE_H

#include <fstream>

void usage(const char *name);

class Arguments {
public:
    Arguments(int argc, char **argv);
    const char *ifname = NULL;
    const char *ofname = "out";
    int bits = 8;
    bool verbose = false;
private:
};

#endif
