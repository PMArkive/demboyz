
#include "netmath.h"

namespace math
{
    uint32_t log2(uint32_t value)
    {
        uint32_t res = 0;
        while (value >>= 1)
            ++res;
        return res;
    }

    uint32_t BitsToBytes(uint32_t bits)
    {
        return ((bits + 7) >> 3);
    }
}
