#include <spdlog/spdlog.h>
#include "dataloader.h"

DataLoader::DataLoader(const char *ifname)
{
    this->ifs.open(ifname, std::fstream::in);
    if (!this->ifs) {
        spdlog::error("Failed to open file: {}, please check if the file exist", ifname);
        this->_ok = false;
    }
}

bool DataLoader::ok()
{
    return this->_ok;
}
