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
