#include "lualib.h"
#include "lapi.h"
#include "lstate.h"
#include <string>
#include <map>
#include <tuple>
#include <time.h>
#include <chrono>

#include "lbit.h"

typedef std::chrono::high_resolution_clock Clock;
#define tonanoseconds(v) std::chrono::duration_cast<std::chrono::nanoseconds>(v);
#define compare(start, end) ((std::chrono::nanoseconds)std::chrono::duration_cast<std::chrono::nanoseconds>(end - start));

#define CODES "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/="
#define CODES_URL_SAFE "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_="

#define EMPTY ""
#define SEP EMPTY

/* macro to `unsign' a character */
#define uchar(c) ((unsigned char)(c))

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

static int base64_encode(lua_State* L)
{
    auto start = Clock::now();
    size_t l;
    const char* str = luaL_optlstring(L, 1, EMPTY, &l);
    bool urlsafe = luaL_optboolean(L, 2, false);
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

        const char* chosen_codes = urlsafe ? CODES_URL_SAFE : CODES;

        // Higher 6 bits of a
        unsigned char ca = string_byte_one(L, chosen_codes, rshift(a, 2));
        // Lower 2 bits of a + high 4 bits of b
        unsigned char cb = string_byte_one(L, chosen_codes, bor(lshift(band(a, 3), 4), ((b != NULL) ? rshift(b, 4) : 0)));
        // Low 4 bits of b + High 2 bits of c
        unsigned char cc = ((b != NULL) ? string_byte_one(L, chosen_codes, bor(lshift(band(b, 15), 2),((c != NULL) ? rshift(c, 6) : 0))) : 61);
        // Lower 6 bits of c
        unsigned char cd = ((c != NULL) ? string_byte_one(L, chosen_codes, band(c, 63)) : 61);

        string_char_four(L, ca, cb, cc, cd);
        lua_rawseti(L, 1, j);

        j++;
        i++;
        i++;
    }
    table_concat(L, 1); // table.concat(parts)
    auto end = Clock::now();
    auto diff = compare(start, end);
    lua_pushllong(L, (long long)diff.count());
    return 2;
}

// Reverse map from character code to 6-bit integer
static std::map<unsigned char, int> map = std::map<unsigned char, int>{}; // map = {}

// Reverse map from character code to 6-bit integer
static std::map<unsigned char, int> map_url_safe = std::map<unsigned char, int>{}; // map_url_safe = {}

static int base64_decode(lua_State* L)
{
    auto start = Clock::now();
    size_t l;
    const char* data = luaL_optlstring(L, 1, EMPTY, &l);
    bool urlsafe = luaL_optboolean(L, 2, false);
    lua_settop(L, 0);
    lua_createtable(L, 0, 0); // bytes = {}
    int j = 1;
    int li = (int)l;
    for (int i = 0; i < li; i++)
    {
        int a = urlsafe ? map_url_safe[string_byte_one(L, data, i)] : map[string_byte_one(L, data, i)]; // a = map[string.byte(data, i)]
        int b = urlsafe ? map_url_safe[string_byte_one(L, data, i + 1)] : map[string_byte_one(L, data, i + 1)]; // b = map[string.byte(data, i + 1)]
        int c = urlsafe ? map_url_safe[string_byte_one(L, data, i + 2)] : map[string_byte_one(L, data, i + 2)]; // c = map[string.byte(data, i + 2)]
        int d = urlsafe ? map_url_safe[string_byte_one(L, data, i + 3)] : map[string_byte_one(L, data, i + 3)]; // d = map[string.byte(data, i + 3)]

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
    auto end = Clock::now();
    auto diff = compare(start, end);
    lua_pushllong(L, (long long)diff.count());
    return 2;
}

#define uint unsigned int

static const char code[]=
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char code_url_safe[]=
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

static void encode(luaL_Buffer *b, uint c1, uint c2, uint c3, int n, bool url_safe)
{
 unsigned long tuple=c3+256UL*(c2+256UL*c1);
 int i;
 char s[4];
 for (i=0; i<4; i++) {
  s[3-i] = (url_safe ? code_url_safe[tuple % 64] : code[tuple % 64]);
  tuple /= 64;
 }
 for (i=n+1; i<4; i++) s[i]='=';
 luaL_addlstring(b,s,4);
}

static int Lencode(lua_State *L)		/** encode(s) */
{
 auto start = Clock::now();
 size_t l;
 const unsigned char *s=(const unsigned char*)luaL_checklstring(L,1,&l);
 bool url_safe = luaL_optboolean(L, 2, false);
 luaL_Buffer b;
 int n;
 luaL_buffinit(L,&b);
 int li = (int)l;
 for (n=li/3; n--; s+=3) encode(&b,s[0],s[1],s[2],3,url_safe);
 switch (li%3)
 {
  case 1: encode(&b,s[0],0,0,1,url_safe);		break;
  case 2: encode(&b,s[0],s[1],0,2,url_safe);		break;
 }
 luaL_pushresult(&b);
 auto end = Clock::now();
 auto diff = compare(start, end);
 lua_pushllong(L, (long long)diff.count());
 return 2;
}

static void decode(luaL_Buffer *b, int c1, int c2, int c3, int c4, int n)
{
 unsigned long tuple=c4+64L*(c3+64L*(c2+64L*c1));
 char s[3];
 switch (--n)
 {
  case 3: s[2]=(char)tuple;
  case 2: s[1]=(char)(tuple >> 8);
  case 1: s[0]=(char)(tuple >> 16);
 }
 luaL_addlstring(b,s,n);
}

static int Ldecode(lua_State *L)		/** decode(s) */
{
  auto start = Clock::now();
 size_t l;
 const char *s=luaL_checklstring(L,1,&l);
 bool url_safe = luaL_optboolean(L, 2, false);
 luaL_Buffer b;
 int n=0;
 char t[4];
 luaL_buffinit(L,&b);
 for (;;)
 {
  int c=*s++;
  switch (c)
  {
   const char *p;
   default:
    p=strchr((url_safe ? code_url_safe : code),c); if (p==NULL) return 0;
    t[n++]= (char)(p-(url_safe ? code_url_safe : code));
    if (n==4)
    {
     decode(&b,t[0],t[1],t[2],t[3],4);
     n=0;
    }
    break;
   case '=':
    switch (n)
    {
     case 1: decode(&b,t[0],0,0,0,1);		break;
     case 2: decode(&b,t[0],t[1],0,0,2);	break;
     case 3: decode(&b,t[0],t[1],t[2],0,3);	break;
    }
   case 0:
    luaL_pushresult(&b);
    auto diff1 = compare(start, Clock::now());
    lua_pushllong(L, (long long)diff1.count());
    return 2;
   case '\n': case '\r': case '\t': case ' ': case '\f': case '\b':
    break;
  }
 }
 auto diff0 = compare(start, Clock::now());
 lua_pushllong(L, (long long)diff0.count());
 return 1;
}

static const luaL_Reg base64lib[] = {
    {"encode", base64_encode},
    {"decode", base64_decode},
    {"encodeL", Lencode},
    {"decodeL", Ldecode},
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

    int lcodes_url = (int)strlen(CODES_URL_SAFE);

    for (int i = 1; i < lcodes_url; i++){
        map_url_safe[string_byte_one(L, CODES_URL_SAFE, i)] = i;
    }

    return 1;
}
