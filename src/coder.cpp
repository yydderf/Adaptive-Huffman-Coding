#include <spdlog/spdlog.h>
#include "dataloader.h"
#include "utils.h"
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

void Coder::write_bits(const boost::dynamic_bitset<> &bits)
{
    // boost::dynamic_bitset<> tmp_repr;
    // std::string repr_buf;

    // boost::to_string(bits, repr_buf);
    // std::reverse(repr_buf.begin(), repr_buf.end());
    // spdlog::info("Encoder write_bits input: {}", repr_buf);

    size_t old_size = this->bit_buf.size();
    this->bit_buf.resize(old_size + bits.size());
    for (size_t i = 0; i < bits.size(); ++i) {
        this->bit_buf[old_size + i] = bits[i];
    }
    while (this->bit_buf.size() >= 8) {
        uint8_t byte = 0;
        for (size_t i = 0; i < 8; ++i) {
            byte = (byte << 1) | (this->bit_buf[i] ? 1 : 0);
        }

        // tmp_repr = this->bit_buf;
        // tmp_repr.resize(8);
        // boost::to_string(tmp_repr, repr_buf);
        // std::reverse(repr_buf.begin(), repr_buf.end());
        // spdlog::info("Encoder write_bits written: {}", repr_buf);

        this->ofs.write(reinterpret_cast<const char*>(&byte), 1);
        erase_left_bits(&(this->bit_buf), 8);
    }

    // tmp_repr = this->bit_buf;
    // boost::to_string(tmp_repr, repr_buf);
    // std::reverse(repr_buf.begin(), repr_buf.end());
    // spdlog::info("Encoder write_bits remaining: {}", repr_buf);
}

void Coder::flush_bits()
{
    std::string repr_buf;
    uint8_t padding_size = 8 - static_cast<uint8_t>(this->bit_buf.size());
    if (this->bit_buf.size() > 0) {
        this->bit_buf.resize(8);
        // size_t remaining = this->bit_buf.size();
        uint8_t byte = 0;
        for (size_t i = 0; i < 8; ++i) {
            byte = (byte << 1) | (this->bit_buf[i] ? 1 : 0);
        }
        this->ofs.write(reinterpret_cast<const char*>(&byte), 1);
        // boost::to_string(this->bit_buf, repr_buf);
        // std::reverse(repr_buf.begin(), repr_buf.end());
        // spdlog::info("Encoder write_bits remaining: {}", repr_buf);
        this->bit_buf.clear();
    }
    // spdlog::warn("padding size: {}", padding_size);
    this->ofs.write(reinterpret_cast<const char*>(&padding_size), 1);
}

void Coder::flush_bits_32()
{
    std::string repr_buf;
    uint8_t padding_size = 32 - static_cast<uint8_t>(this->bit_buf.size());
    boost::to_string(this->bit_buf, repr_buf);
    std::reverse(repr_buf.begin(), repr_buf.end());
    spdlog::warn("Encoder write_bits remaining: {}", repr_buf);
    if (this->bit_buf.size() > 0) {
        this->bit_buf.resize(32);
        // size_t remaining = this->bit_buf.size();
        uint32_t byte = 0;
        for (size_t i = 0; i < 32; ++i) {
            byte = (byte << 1) | (this->bit_buf[i] ? 1 : 0);
        }
        this->ofs.write(reinterpret_cast<const char*>(&byte), 4);
        this->bit_buf.clear();
    }
    // spdlog::warn("padding size: {}", padding_size);
    this->ofs.write(reinterpret_cast<const char*>(&padding_size), 1);
}

bool Coder::read_bit(bool &bit)
{
    if (this->bit_buf.empty()) {
        uint8_t byte;
        if (!this->read_byte(byte)) {
            return false;
        }
        boost::dynamic_bitset<> tmp(8, static_cast<unsigned long long>(byte));
        for (size_t i = 0; i < 8; ++i) {
            this->bit_buf.push_back(tmp.test(7 - i));
        }
    }
    bit = this->bit_buf[0];
    // remove the first bit from the bitset
    // this->bit_buf.erase(this->bit_buf.begin());
    erase_left_bits(&(this->bit_buf), 1);
    return true;
}

boost::dynamic_bitset<> Coder::read_bits(int n)
{
    boost::dynamic_bitset<> bits(n);
    for (int i = 0; i < n; ++i) {
        bool b;
        if (!this->read_bit(b)) {
            break;
        }
        bits[n - 1 - i] = b;
    }
    return bits;
}

boost::dynamic_bitset<> Coder::read_bits_rev(int n)
{
    boost::dynamic_bitset<> bits(n);
    for (int i = 0; i < n; ++i) {
        bool b;
        if (!this->read_bit(b)) {
            break;
        }
        bits[i] = b;
    }
    return bits;
}

bool Coder::read_byte(uint8_t &byte)
{
    std::streamsize read_bytes;
    std::vector<char> *data = this->dl->get(1, &read_bytes);
    if (data == nullptr || read_bytes < 1) {
        return false;
    }
    byte = static_cast<uint8_t>((*data)[0]);
    return true;
}
