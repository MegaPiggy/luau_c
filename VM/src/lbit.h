

#define ALLONES ~0u
#define NBITS int(8 * sizeof(unsigned))

/* macro to trim extra bits */
#define trim(x) ((x)&ALLONES)

/* builds a number with 'n' ones (1 <= n <= NBITS) */
#define mask(n) (~((ALLONES << 1) << ((n)-1)))

typedef unsigned b_uint;

b_uint band(b_uint a, b_uint b);
b_uint bor(b_uint a, b_uint b);
b_uint bxor(b_uint a, b_uint b);
b_uint bnot(b_uint a);
b_uint shift(b_uint r, int i);
b_uint lshift(b_uint r, int i);
b_uint rshift(b_uint r, int i);