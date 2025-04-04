#ifndef _UTILS_H
#define _UTILS_H

#include <cstdint>
#include <boost/dynamic_bitset.hpp>

boost::dynamic_bitset<> int_to_bitset(uint32_t val, int num_bits);
uint32_t bitset_to_int(const boost::dynamic_bitset<> &bits);
void erase_left_bits(boost::dynamic_bitset<> *bits, size_t n);

#endif
