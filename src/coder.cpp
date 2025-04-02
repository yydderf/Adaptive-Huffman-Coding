#include <spdlog/spdlog.h>
#include "coder.h"

Coder::Coder(DataLoader *dl, int bits, const char *ofname)
{
    this->ofs.open(ofname, std::fstream::out);
    if (!this->ofs) {
        spdlog::error("Failed to open output file: {}, check if there's any permission issue", ofname);
    }
    this->dl = dl;
    this->bits = bits;
}

int Coder::proc()
{
    // while (!this->dl->eof()) {
    //     read the populated data from data loader
    //     std::unique_ptr<RawBlock> raw_block = this->dl->next();
    //     spdlog::debug("raw block size: {}", raw_block->size);
    // }
    return 0;
}
