#include "utils.h"
#include <cstdint>
#include <boost/dynamic_bitset.hpp>

boost::dynamic_bitset<> int_to_bitset(uint32_t val, int num_bits)
{
    boost::dynamic_bitset<> bits(num_bits);
    for (int i = 0; i < num_bits; ++i) {
        bits[num_bits - 1 - i] = (val >> i) & 1;
    }
    return bits;
}

uint32_t bitset_to_int(const boost::dynamic_bitset<> &bits)
{
    uint32_t val = 0;
    for (size_t i = 0; i < bits.size(); ++i) {
        // val = (val << 1) | (bits.test(i) ? 1 : 0);
        if (bits.test(i)) val |= (1U << i);
    }
    return val;
}

void erase_left_bits(boost::dynamic_bitset<> *bits, size_t n)
{
    if (!bits->empty() && n > 0 && bits->size() >= n) {
        for (size_t i = n; i < bits->size(); ++i) {
            (*bits)[i - n] = (*bits)[i];
        }
        bits->resize(bits->size() - n);
    }
}
