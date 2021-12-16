#include "lbit.h"

b_uint band(b_uint a, b_uint b)
{
    b_uint r = ~(b_uint)0;
    r &= a;
    r &= b;
    return trim(r);
}

b_uint bor(b_uint a, b_uint b)
{
    b_uint r = (b_uint)0;
    r |= a;
    r |= b;
    return trim(r);
}

b_uint bxor(b_uint a, b_uint b)
{
    b_uint r = (b_uint)0;
    r ^= a;
    r ^= b;
    return trim(r);
}

b_uint bnot(b_uint a)
{
    b_uint r = ~a;
    return trim(r);
}

b_uint shift(b_uint r, int i)
{
    if (i < 0)
    { /* shift right? */
        i = -i;
        r = trim(r);
        if (i >= NBITS)
            r = 0;
        else
            r >>= i;
    }
    else
    { /* shift left */
        if (i >= NBITS)
            r = 0;
        else
            r <<= i;
        r = trim(r);
    }
    return r;
}

b_uint lshift(b_uint r, int i)
{
    return shift(r, i);
}

b_uint rshift(b_uint r, int i)
{
    return shift(r, -i);
}