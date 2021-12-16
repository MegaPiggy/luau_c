#include "lboost.h"

#define api_incr_top(L) \
    { \
        api_check(L, L->top < L->ci->top); \
        L->top++; \
    }

static l_noret tag_error(lua_State* L, int narg, int tag)
{
    luaL_typeerrorL(L, narg, lua_typename(L, tag));
}

boost::multiprecision::int128_t luaL_checkint128(lua_State* L, int narg)
{
    int isnum;
    boost::multiprecision::int128_t d = lua_toint128x(L, narg, &isnum);
    if (!isnum)
        tag_error(L, narg, LUA_TNUMBER);
    return d;
}

boost::multiprecision::int128_t luaL_optint128(lua_State* L, int narg, boost::multiprecision::int128_t def)
{
    return luaL_opt(L, luaL_checkint128, narg, def);
}

boost::multiprecision::int256_t luaL_checkint256(lua_State* L, int narg)
{
    int isnum;
    boost::multiprecision::int256_t d = lua_toint256x(L, narg, &isnum);
    if (!isnum)
        tag_error(L, narg, LUA_TNUMBER);
    return d;
}

boost::multiprecision::int256_t luaL_optint256(lua_State* L, int narg, boost::multiprecision::int256_t def)
{
    return luaL_opt(L, luaL_checkint256, narg, def);
}

boost::multiprecision::int512_t luaL_checkint512(lua_State* L, int narg)
{
    int isnum;
    boost::multiprecision::int512_t d = lua_toint512x(L, narg, &isnum);
    if (!isnum)
        tag_error(L, narg, LUA_TNUMBER);
    return d;
}

boost::multiprecision::int512_t luaL_optint512(lua_State* L, int narg, boost::multiprecision::int512_t def)
{
    return luaL_opt(L, luaL_checkint512, narg, def);
}

boost::multiprecision::int1024_t luaL_checkint1024(lua_State* L, int narg)
{
    int isnum;
    boost::multiprecision::int1024_t d = lua_toint1024x(L, narg, &isnum);
    if (!isnum)
        tag_error(L, narg, LUA_TNUMBER);
    return d;
}

boost::multiprecision::int1024_t luaL_optint1024(lua_State* L, int narg, boost::multiprecision::int1024_t def)
{
    return luaL_opt(L, luaL_checkint1024, narg, def);
}

boost::multiprecision::uint128_t luaL_checkuint128(lua_State* L, int narg)
{
    int isnum;
    boost::multiprecision::uint128_t d = lua_touint128x(L, narg, &isnum);
    if (!isnum)
        tag_error(L, narg, LUA_TNUMBER);
    return d;
}

boost::multiprecision::uint128_t luaL_optuint128(lua_State* L, int narg, boost::multiprecision::uint128_t def)
{
    return luaL_opt(L, luaL_checkuint128, narg, def);
}

boost::multiprecision::uint256_t luaL_checkuint256(lua_State* L, int narg)
{
    int isnum;
    boost::multiprecision::uint256_t d = lua_touint256x(L, narg, &isnum);
    if (!isnum)
        tag_error(L, narg, LUA_TNUMBER);
    return d;
}

boost::multiprecision::uint256_t luaL_optuint256(lua_State* L, int narg, boost::multiprecision::uint256_t def)
{
    return luaL_opt(L, luaL_checkuint256, narg, def);
}

boost::multiprecision::uint512_t luaL_checkuint512(lua_State* L, int narg)
{
    int isnum;
    boost::multiprecision::uint512_t d = lua_touint512x(L, narg, &isnum);
    if (!isnum)
        tag_error(L, narg, LUA_TNUMBER);
    return d;
}

boost::multiprecision::uint512_t luaL_optuint512(lua_State* L, int narg, boost::multiprecision::uint512_t def)
{
    return luaL_opt(L, luaL_checkuint512, narg, def);
}

boost::multiprecision::uint1024_t luaL_checkuint1024(lua_State* L, int narg)
{
    int isnum;
    boost::multiprecision::uint1024_t d = lua_touint1024x(L, narg, &isnum);
    if (!isnum)
        tag_error(L, narg, LUA_TNUMBER);
    return d;
}

boost::multiprecision::uint1024_t luaL_optuint1024(lua_State* L, int narg, boost::multiprecision::uint1024_t def)
{
    return luaL_opt(L, luaL_checkuint1024, narg, def);
}

void lua_pushint128(lua_State* L, boost::multiprecision::int128_t n)
{
    setnvalue(L->top, cast_num(n));
    api_incr_top(L);
    return;
}

void lua_pushint256(lua_State* L, boost::multiprecision::int256_t n)
{
    setnvalue(L->top, cast_num(n));
    api_incr_top(L);
    return;
}

void lua_pushint512(lua_State* L, boost::multiprecision::int512_t n)
{
    setnvalue(L->top, cast_num(n));
    api_incr_top(L);
    return;
}

void lua_pushint1024(lua_State* L, boost::multiprecision::int1024_t n)
{
    setnvalue(L->top, cast_num(n));
    api_incr_top(L);
    return;
}

void lua_pushuint128(lua_State* L, boost::multiprecision::uint128_t n)
{
    setnvalue(L->top, cast_num(n));
    api_incr_top(L);
    return;
}

void lua_pushuint256(lua_State* L, boost::multiprecision::uint256_t n)
{
    setnvalue(L->top, cast_num(n));
    api_incr_top(L);
    return;
}

void lua_pushuint512(lua_State* L, boost::multiprecision::uint512_t n)
{
    setnvalue(L->top, cast_num(n));
    api_incr_top(L);
    return;
}

void lua_pushuint1024(lua_State* L, boost::multiprecision::uint1024_t n)
{
    setnvalue(L->top, cast_num(n));
    api_incr_top(L);
    return;
}

boost::multiprecision::int128_t lua_toint128x(lua_State* L, int idx, int* isnum)
{
    TValue n;
    const TValue* o = luaA_index2adr(L, idx);
    if (tonumber(o, &n))
    {
        double num = nvalue(o);
        if (isnum)
            *isnum = 1;
        return boost::multiprecision::int128_t(num);
    }
    else
    {
        if (isnum)
            *isnum = 0;
        return  boost::multiprecision::int128_t(0);
    }
}

boost::multiprecision::uint128_t lua_touint128x(lua_State* L, int idx, int* isnum)
{
    TValue n;
    const TValue* o = luaA_index2adr(L, idx);
    if (tonumber(o, &n))
    {
        double num = nvalue(o);
        if (isnum)
            *isnum = 1;
        return boost::multiprecision::uint128_t(num);
    }
    else
    {
        if (isnum)
            *isnum = 0;
        return boost::multiprecision::uint128_t(0);
    }
}

boost::multiprecision::int256_t lua_toint256x(lua_State* L, int idx, int* isnum)
{
    TValue n;
    const TValue* o = luaA_index2adr(L, idx);
    if (tonumber(o, &n))
    {
        double num = nvalue(o);
        if (isnum)
            *isnum = 1;
        return boost::multiprecision::int256_t(num);
    }
    else
    {
        if (isnum)
            *isnum = 0;
        return boost::multiprecision::int256_t(0);
    }
}

boost::multiprecision::uint256_t lua_touint256x(lua_State* L, int idx, int* isnum)
{
    TValue n;
    const TValue* o = luaA_index2adr(L, idx);
    if (tonumber(o, &n))
    {
        double num = nvalue(o);
        if (isnum)
            *isnum = 1;
        return boost::multiprecision::uint256_t(num);
    }
    else
    {
        if (isnum)
            *isnum = 0;
        return boost::multiprecision::uint256_t(0);
    }
}

boost::multiprecision::int512_t lua_toint512x(lua_State* L, int idx, int* isnum)
{
    TValue n;
    const TValue* o = luaA_index2adr(L, idx);
    if (tonumber(o, &n))
    {
        double num = nvalue(o);
        if (isnum)
            *isnum = 1;
        return boost::multiprecision::int512_t(num);
    }
    else
    {
        if (isnum)
            *isnum = 0;
        return boost::multiprecision::int512_t(0);
    }
}

boost::multiprecision::uint512_t lua_touint512x(lua_State* L, int idx, int* isnum)
{
    TValue n;
    const TValue* o = luaA_index2adr(L, idx);
    if (tonumber(o, &n))
    {
        double num = nvalue(o);
        if (isnum)
            *isnum = 1;
        return boost::multiprecision::uint512_t(num);
    }
    else
    {
        if (isnum)
            *isnum = 0;
        return boost::multiprecision::uint512_t(0);
    }
}

boost::multiprecision::int1024_t lua_toint1024x(lua_State* L, int idx, int* isnum)
{
    TValue n;
    const TValue* o = luaA_index2adr(L, idx);
    if (tonumber(o, &n))
    {
        double num = nvalue(o);
        if (isnum)
            *isnum = 1;
        return boost::multiprecision::int1024_t(num);
    }
    else
    {
        if (isnum)
            *isnum = 0;
        return boost::multiprecision::int1024_t(0);
    }
}

boost::multiprecision::uint1024_t lua_touint1024x(lua_State* L, int idx, int* isnum)
{
    TValue n;
    const TValue* o = luaA_index2adr(L, idx);
    if (tonumber(o, &n))
    {
        double num = nvalue(o);
        if (isnum)
            *isnum = 1;
        return boost::multiprecision::uint1024_t(num);
    }
    else
    {
        if (isnum)
            *isnum = 0;
        return boost::multiprecision::uint1024_t(0);
    }
}