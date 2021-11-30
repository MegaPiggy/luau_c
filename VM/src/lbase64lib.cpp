#include "lualib.h"
#include "lapi.h"
#include "lstate.h"
#include <string>
#include <map>
#include <tuple>

#define ALLONES ~0u
#define NBITS int(8 * sizeof(unsigned))

/* macro to trim extra bits */
#define trim(x) ((x)&ALLONES)

/* builds a number with 'n' ones (1 <= n <= NBITS) */
#define mask(n) (~((ALLONES << 1) << ((n)-1)))

#define CODES "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/="
#define EMPTY ""
#define SEP EMPTY

/* macro to `unsign' a character */
#define uchar(c) ((unsigned char)(c))

typedef unsigned b_uint;

static void addfield(lua_State* L, int idx, luaL_Buffer* b, int i)
{
    lua_rawgeti(L, idx, i);
    if (!lua_isstring(L, -1))
        luaL_error(L, "invalid value (%s) at index %d in table for 'concat'", luaL_typename(L, -1), i);
    luaL_addvalue(b);
}

static int table_concat(lua_State* L, int idx)
{
    luaL_checktype(L, idx, LUA_TTABLE);
    luaL_Buffer b;
    int i = 1;
    int last = lua_objlen(L, idx);
    luaL_buffinit(L, &b);
    for (; i < last; i++)
    {
        addfield(L, idx, &b, i);
        luaL_addstring(&b, SEP);
    }
    if (i == last) /* add last value (if interval was not empty) */
        addfield(L, idx, &b, i);
    luaL_pushresult(&b);
    return 1;
}

static int posrelat(int pos, size_t len)
{
    /* relative string position: negative means back from end */
    if (pos < 0)
        pos += (int)len + 1;
    return (pos >= 0) ? pos : 0;
}

static std::tuple<unsigned char, unsigned char, unsigned char> string_byte(lua_State* L, const char* s, size_t l, int i = NULL, int e = NULL)
{
    int posi = posrelat(i, l);
    int pose = posrelat(e, l);

    int n;
    if (posi < 0)
        posi = 0;

    if ((size_t)pose > l)
        pose = (int)l;

    if (posi > pose)
        return std::tuple<unsigned char, unsigned char, unsigned char>(NULL, NULL, NULL); /* empty interval; return no values */

    n = (int)(pose - posi + 1);
    if (posi + n <= pose) /* overflow? */{
        luaL_error(L, "string slice too long");
        return std::tuple<unsigned char, unsigned char, unsigned char>(NULL, NULL, NULL);
    }
    
    unsigned char a = uchar(s[posi]);
    if (n == 1)
        return std::tuple<unsigned char, unsigned char, unsigned char>(a, NULL, NULL);
    
    unsigned char b = uchar(s[posi + 1]);
    if (n == 2)
        return std::tuple<unsigned char, unsigned char, unsigned char>(a, b, NULL);

    unsigned char c = uchar(s[posi + 2]);
    return std::tuple<unsigned char, unsigned char, unsigned char>(a, b, c);
}

static unsigned char string_byte_one(lua_State* L, const char* s, int i = 0)
{
    size_t l = strlen(s);
    int posi = posrelat(i, l);
    int pose = posrelat(posi, l);
    
    if (posi < 0)
        posi = 0;

    if ((size_t)pose > l)
        pose = (int)l;

    if (posi > pose)
        return NULL; /* empty interval; return no values */

    return uchar(s[posi]);
}

static int string_char_one(lua_State* L, b_uint c)
{
    if (uchar(c) != c){
        luaL_error(L, "invalid value");
        return 0;
    }

    luaL_Buffer b;
    char* ptr = luaL_buffinitsize(L, &b, 1);

    *ptr++ = uchar(c);

    luaL_pushresultsize(&b, 1);

    //const char* chr = lua_tostring(L, -1);
    //lua_pop(L, 1);

    return 1;
}

static int string_char_four(lua_State* L, int ca, int cb, int cc, int cd)
{
    if (uchar(ca) != ca || uchar(cb) != cb || uchar(cc) != cc || uchar(cd) != cd){
        luaL_error(L, "invalid value");
        return 0;
    }

    luaL_Buffer b;
    char* ptr = luaL_buffinitsize(L, &b, 4);

    *ptr++ = uchar(ca);
    *ptr++ = uchar(cb);
    *ptr++ = uchar(cc);
    *ptr++ = uchar(cd);

    luaL_pushresultsize(&b, 4);

    return 1;
}

static b_uint band(b_uint a, b_uint b)
{
    b_uint r = ~(b_uint)0;
    r &= a;
    r &= b;
    return trim(r);
}

static b_uint bor(b_uint a, b_uint b)
{
    b_uint r = (b_uint)0;
    r |= a;
    r |= b;
    return trim(r);
}

static b_uint shift(b_uint r, int i)
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

static b_uint lshift(b_uint r, int i)
{
    return shift(r, i);
}

static b_uint rshift(b_uint r, int i)
{
    return shift(r, -i);
}

static int base64_encode(lua_State* L)
{
    size_t l;
    const char* str = luaL_optlstring(L, 1, EMPTY, &l);
    lua_settop(L, 0);
    lua_createtable(L, 0, 0); // parts = {}
    int j = 1;
    int li = (int)l;
    for (int i = 0; i < li; i++)
    {
        std::tuple<unsigned char,unsigned char,unsigned char> abc = string_byte(L, str, l, i, i + 2); // local a, b, c = byte(str, i, i + 2)
        unsigned char a = std::get<0>(abc);
        unsigned char b = std::get<1>(abc);
        unsigned char c = std::get<2>(abc);

        // 61 is '='

        // Higher 6 bits of a
        unsigned char ca = string_byte_one(L, CODES, rshift(a, 2));
        // Lower 2 bits of a + high 4 bits of b
        unsigned char cb = string_byte_one(L, CODES, bor(lshift(band(a, 3), 4), ((b != NULL) ? rshift(b, 4) : 0)));
        // Low 4 bits of b + High 2 bits of c
        unsigned char cc = ((b != NULL) ? string_byte_one(L, CODES, bor(lshift(band(b, 15), 2),((c != NULL) ? rshift(c, 6) : 0))) : 61);
        // Lower 6 bits of c
        unsigned char cd = ((c != NULL) ? string_byte_one(L, CODES, band(c, 63)) : 61);

        string_char_four(L, ca, cb, cc, cd);
        lua_rawseti(L, 1, j);

        j++;
        i++;
        i++;
    }
    table_concat(L, 1); // table.concat(parts)
    return 1;
}

// Reverse map from character code to 6-bit integer
static std::map<unsigned char, int> map = std::map<unsigned char, int>{}; // map = {}

static int base64_decode(lua_State* L)
{
    size_t l;
    const char* data = luaL_optlstring(L, 1, EMPTY, &l);
    lua_settop(L, 0);
    lua_createtable(L, 0, 0); // bytes = {}
    int j = 1;
    int li = (int)l;
    for (int i = 0; i < li; i++)
    {
        int a = map[string_byte_one(L, data, i)]; // a = map[string.byte(data, i)]
        int b = map[string_byte_one(L, data, i + 1)]; // b = map[string.byte(data, i + 1)]
        int c = map[string_byte_one(L, data, i + 2)]; // c = map[string.byte(data, i + 2)]
        int d = map[string_byte_one(L, data, i + 3)]; // d = map[string.byte(data, i + 3)]

        // higher 6 bits are the first char
        // lower 2 bits are upper 2 bits of second char
        string_char_one(L, bor(lshift(a, 2), rshift(b, 4)));
        lua_rawseti(L, 1, j); /* bytes[j] = string.char(bit32.bor(bit32.lshift(a, 2), bit32.rshift(b, 4))) */

        // if the third char is not padding, we have a second byte
        if (c < 64){
            // high 4 bits come from lower 4 bits in b
            // low 4 bits come from high 4 bits in c
            string_char_one(L, bor(lshift(band(b, 0xf), 4), rshift(c, 2)));
            lua_rawseti(L, 1, j + 1); /* bytes[j + 1] = string.char(bit32.bor(bit32.lshift(bit32.band(b, 0xf), 4), bit32.rshift(c, 2))) */

            // if the fourth char is not padding, we have a third byte
            if (d < 64) {
                // Upper 2 bits come from Lower 2 bits of c
                // Lower 6 bits come from d
                string_char_one(L, bor(lshift(band(c, 3), 6), d));
                lua_rawseti(L, 1, j + 2); /* bytes[j + 2] = string.char(bit32.bor(bit32.lshift(bit32.band(c, 3), 6), d)) */
            }
        }

        j = j + 3;
        i++;
        i++;
        i++;
    }
    table_concat(L, 1); // table.concat(bytes)
    return 1;
}

static const luaL_Reg base64lib[] = {
    {"encode", base64_encode},
    {"decode", base64_decode},
    {NULL, NULL},
};

/*
** Open base64 library
*/
LUALIB_API int luaopen_base64(lua_State* L)
{
    luaL_register(L, LUA_BASE64LIBNAME, base64lib);

    int lcodes = (int)strlen(CODES);

    for (int i = 1; i < lcodes; i++){
        map[string_byte_one(L, CODES, i)] = i;
    }

    return 1;
}
