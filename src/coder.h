#ifndef _CODER_H
#define _CODER_H

#include "dataloader.h"
#include <fstream>

class Coder {
public:
    Coder(DataLoader *dl, int bits, const char *ofname);
    int proc();
protected:
    DataLoader *dl;
    std::ofstream ofs;
    int bits;
};

#endif
