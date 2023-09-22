#include "CommHelper.h"
uint32_t calculate_pad_len(uint32_t len, uint32_t type_size, uint32_t num_dpus){
    uint64_t len_in_byte = (uint64_t)len*type_size;

    // calculate lcm of typesize and 8, each dpu gets %8
    uint32_t lcm = (type_size > 8) ? type_size : 8;

    while (1) {
        if (lcm % type_size == 0 && lcm % 8 == 0) {
            break;
        }
        ++lcm;
    }

    // divisible by typesize
    uint64_t padded_len = len_in_byte;

    while (1) {
        if (padded_len % num_dpus == 0 && (padded_len/num_dpus) % lcm == 0) {
            break;
        }
        ++padded_len;
    }

    uint64_t pad_len = padded_len - len_in_byte;
    return (uint32_t)pad_len;
}
