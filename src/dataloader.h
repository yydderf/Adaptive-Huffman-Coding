#ifndef _DATALOADER_H
#define _DATALOADER_H

#include <fstream>
#include <unordered_map>

enum DL_MODE {
    BLOCK,
    ONCE,
};

class DataLoader {
public:
    DataLoader(const char *ifname);
    bool ok();
private:
    std::ifstream ifs;
    bool _ok = true;
    // buf
    std::unordered_map<uint32_t, size_t> buf_map;
    std::unordered_map<uint32_t, size_t> freq_map;
};

#endif
