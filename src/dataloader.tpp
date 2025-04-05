#include <iostream>
#include <fstream>
#include <spdlog/spdlog.h>
#include <cstring>

template <typename T>
std::unique_ptr<RawBlock<T>> DataLoader::next() {
    auto raw_block = std::make_unique<RawBlock<T>>();

    if (this->_eof) {
        spdlog::warn("No more data to be read");
        return nullptr;
    }

    // Calculate how many elements of type T can be read.
    size_t element_size = sizeof(T);
    size_t num_elements = this->file_size / element_size;

    // Resize the vector to hold the elements.
    try {
        raw_block->raw_data.resize(num_elements);
    } catch (const std::bad_alloc& e) {
        spdlog::error("Not enough memory, try reading the file block by block: {}", e.what());
        return nullptr;
    }

    // Read data from file.
    this->ifs.read(reinterpret_cast<char*>(raw_block->raw_data.data()), num_elements * element_size);
    std::streamsize bytes_read = this->ifs.gcount();

    // Update the number of elements actually read.
    raw_block->size = bytes_read / element_size;

    // Generate frequency table.
    for (size_t i = 0; i < raw_block->size; ++i) {
        raw_block->freq_map[raw_block->raw_data[i]]++;
    }

    // If fewer bytes were read than expected, mark EOF.
    if (static_cast<size_t>(bytes_read) < num_elements * element_size) {
        this->_eof = true;
    }

    return raw_block;
}

