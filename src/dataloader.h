#ifndef _DATALOADER_H
#define _DATALOADER_H

#include <fstream>
#include <unordered_map>
#include <memory>

enum DL_MODE {
    READ_BLOCK = 1L << 0,
    READ_ONCE  = 1L << 1,
    MODE_ENC   = 1L << 2,
    MODE_DEC   = 1L << 3,
};

struct RawBlock {
    uint8_t tag; // header or data
    std::size_t size; // size of block
    std::vector<uint8_t> raw_bytes; // raw data
    std::unordered_map<uint32_t, size_t> freq_map;
};

class DataLoader {
public:
    DataLoader(const char *ifname, int bits, int mode);
    bool ok();
    bool eof();
    std::unique_ptr<RawBlock> next();
private:
    std::ifstream ifs;
    std::size_t read_block_size = 1024;
    std::size_t freq_block_size = 1024;
    std::size_t file_size = 0;
    int _mode;
    int _bits;
    bool _ok = true;
    bool _eof = false;
};

#endif
