#ifndef _CODER_H
#define _CODER_H

#include "dataloader.h"
#include <fstream>

class Coder {
public:
    Coder(DataLoader *dl, int bits, const char *ofname);
private:
    std::ofstream ofs;
    DataLoader *dl;
    int bits;
};

#endif
