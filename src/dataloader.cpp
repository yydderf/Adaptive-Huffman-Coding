#include <spdlog/spdlog.h>
#include <vector>

#include "dataloader.h"

DataLoader::DataLoader(const char *ifname, int bits, int mode)
{
    this->ifs.open(ifname, std::fstream::in | std::ifstream::binary);
    if (!this->ifs) {
        spdlog::error("Failed to open file: {}, please check if the file exist", ifname);
        this->_ok = false;
    }

    this->_mode = mode;
    this->_bits = bits;
    this->buf.resize(this->buf_size);

    this->ifs.seekg(0, this->ifs.end);
    this->file_size = this->ifs.tellg();
    this->ifs.seekg(0, this->ifs.beg);
    spdlog::debug("Input file size is: {}", this->file_size);
    spdlog::info("DataLoader Initialized");
}

bool DataLoader::ok()
{
    return this->_ok;
}

bool DataLoader::eof()
{
    return this->_eof;
}

std::vector<char> *DataLoader::get(ssize_t nbytes, std::streamsize *read_bytes)
{
    // return a pointer to n raw bytes
    // 1. allocate n bytes of buf
    // 2. read data from file to buf
    // 3. return the pointer to buf
    // TODO
    // 1. read more size to reduce I/O frequency
    if (this->_eof == true) {
        spdlog::warn("No more data to be read");
        return nullptr;
    }

    // allocate more space for vector
    if (nbytes > this->buf_size) {
        ssize_t tmp_size = this->buf_size;
        while (nbytes > tmp_size) {
            tmp_size *= 2;
        }
        try {
            this->buf.resize(tmp_size);
            this->buf_size = tmp_size;
        } catch (std::bad_alloc const &) {
            spdlog::error("Not enough memory, try smaller size");
            return nullptr;
        }
    }
    
    this->ifs.read(this->buf.data(), nbytes);
    *read_bytes = this->ifs.gcount();

    if (*read_bytes == nbytes) {
        spdlog::debug("Read {} bytes from file", nbytes);
    } else {
        spdlog::debug("Not enough data in file, only read {} bytes from file", *read_bytes);
        this->_eof = true;
    }

    return &(this->buf);
}

// populate raw block
std::unique_ptr<RawBlock> DataLoader::next()
{
    auto raw_block = std::unique_ptr<RawBlock>(new RawBlock {
        0, 0, std::vector<uint8_t>(this->read_block_size), std::unordered_map<uint32_t, size_t>()
    });

    // eof guard
    if (this->_eof == true) {
        spdlog::warn("No more data to be read");
        return nullptr;
    }

    // proc header for each block
    if (this->_mode & MODE_DEC) {
    }

    if (this->_mode & READ_BLOCK) {
    } else {
        // populate the buffer entirely
        // resize the buffer
        try {
            raw_block->raw_bytes.resize(this->file_size);
        } catch (std::bad_alloc const &) {
            spdlog::error("Not enough memory, try reading the file block by block");
            return nullptr;
        }
        // read from file and write to buffer
        while (raw_block->size < this->file_size && this->ifs) {
            this->ifs.read(
                reinterpret_cast<char*>(raw_block->raw_bytes.data() + raw_block->size),
                this->read_block_size
            );
            std::streamsize nread = this->ifs.gcount();
            if (nread <= 0) {
                spdlog::error("Failed to read from file");
                break;
            }
            raw_block->size += nread;
            spdlog::debug(nread);
            // spdlog::debug("current size: {}", raw_block->size);
        }
    }

    // generate frequency table
    size_t i;
    if (this->_bits == 8)  {
        for (i = 0; i < raw_block->size; ++i) {
            raw_block->freq_map[raw_block->raw_bytes[i]]++;
        }
    }


    return raw_block;
}
