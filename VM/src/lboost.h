#include "lcommon.h"
#include "lua.h"
#include "lualib.h"
#include "lapi.h"
#include "lobject.h"
#include "lstate.h"
#include "lvm.h"
#include <boost/multiprecision/cpp_int.hpp>

using int2048_t = boost::multiprecision::number<boost::multiprecision::backends::cpp_int_backend<2048, 2048, boost::multiprecision::signed_magnitude, boost::multiprecision::unchecked, void> >;
using uint2048_t = boost::multiprecision::number<boost::multiprecision::backends::cpp_int_backend<2048, 2048, boost::multiprecision::unsigned_magnitude, boost::multiprecision::unchecked, void> >;

#define cast_int128(i) cast_to(boost::multiprecision::int128_t, (i))
#define cast_int256(i) cast_to(boost::multiprecision::int256_t, (i))
#define cast_int512(i) cast_to(boost::multiprecision::int512_t, (i))
#define cast_int1024(i) cast_to(boost::multiprecision::int1024_t, (i))
#define cast_uint128(i) cast_to(boost::multiprecision::uint128_t, (i))
#define cast_uint256(i) cast_to(boost::multiprecision::uint256_t, (i))
#define cast_uint512(i) cast_to(boost::multiprecision::uint512_t, (i))
#define cast_uint1024(i) cast_to(boost::multiprecision::uint1024_t, (i))

LUA_API boost::multiprecision::int128_t lua_toint128x(lua_State* L, int idx, int* isnum);
LUA_API boost::multiprecision::int256_t lua_toint256x(lua_State* L, int idx, int* isnum);
LUA_API boost::multiprecision::int512_t lua_toint512x(lua_State* L, int idx, int* isnum);
LUA_API boost::multiprecision::int1024_t lua_toint1024x(lua_State* L, int idx, int* isnum);
LUA_API boost::multiprecision::uint128_t lua_touint128x(lua_State* L, int idx, int* isnum);
LUA_API boost::multiprecision::uint256_t lua_touint256x(lua_State* L, int idx, int* isnum);
LUA_API boost::multiprecision::uint512_t lua_touint512x(lua_State* L, int idx, int* isnum);
LUA_API boost::multiprecision::uint1024_t lua_touint1024x(lua_State* L, int idx, int* isnum);

LUA_API void lua_pushint128(lua_State* L, boost::multiprecision::int128_t n);
LUA_API void lua_pushint256(lua_State* L, boost::multiprecision::int256_t n);
LUA_API void lua_pushint512(lua_State* L, boost::multiprecision::int512_t n);
LUA_API void lua_pushint1024(lua_State* L, boost::multiprecision::int1024_t n);
LUA_API void lua_pushuint128(lua_State* L, boost::multiprecision::uint128_t n);
LUA_API void lua_pushuint256(lua_State* L, boost::multiprecision::uint256_t n);
LUA_API void lua_pushuint512(lua_State* L, boost::multiprecision::uint512_t n);
LUA_API void lua_pushuint1024(lua_State* L, boost::multiprecision::uint1024_t n);

LUALIB_API boost::multiprecision::int128_t luaL_checkint128(lua_State* L, int narg);
LUALIB_API boost::multiprecision::int128_t luaL_optint128(lua_State* L, int narg, boost::multiprecision::int128_t def);
LUALIB_API boost::multiprecision::int256_t luaL_checkint256(lua_State* L, int narg);
LUALIB_API boost::multiprecision::int256_t luaL_optint256(lua_State* L, int narg, boost::multiprecision::int256_t def);
LUALIB_API boost::multiprecision::int512_t luaL_checkint512(lua_State* L, int narg);
LUALIB_API boost::multiprecision::int512_t luaL_optint512(lua_State* L, int narg, boost::multiprecision::int512_t def);
LUALIB_API boost::multiprecision::int1024_t luaL_checkint1024(lua_State* L, int narg);
LUALIB_API boost::multiprecision::int1024_t luaL_optint1024(lua_State* L, int narg, boost::multiprecision::int1024_t def);
LUALIB_API boost::multiprecision::uint128_t luaL_checkuint128(lua_State* L, int narg);
LUALIB_API boost::multiprecision::uint128_t luaL_optuint128(lua_State* L, int narg, boost::multiprecision::uint128_t def);
LUALIB_API boost::multiprecision::uint256_t luaL_checkuint256(lua_State* L, int narg);
LUALIB_API boost::multiprecision::uint256_t luaL_optuint256(lua_State* L, int narg, boost::multiprecision::uint256_t def);
LUALIB_API boost::multiprecision::uint512_t luaL_checkuint512(lua_State* L, int narg);
LUALIB_API boost::multiprecision::uint512_t luaL_optuint512(lua_State* L, int narg, boost::multiprecision::uint512_t def);
LUALIB_API boost::multiprecision::uint1024_t luaL_checkuint1024(lua_State* L, int narg);
LUALIB_API boost::multiprecision::uint1024_t luaL_optuint1024(lua_State* L, int narg, boost::multiprecision::uint1024_t def);

#define lua_toint128(L, i) lua_toint128x(L, i, NULL)
#define lua_toint256(L, i) lua_toint256x(L, i, NULL)
#define lua_toint512(L, i) lua_toint512x(L, i, NULL)
#define lua_toint1024(L, i) lua_toint1024x(L, i, NULL)
#define lua_touint128(L, i) lua_touint128x(L, i, NULL)
#define lua_touint256(L, i) lua_touint256x(L, i, NULL)
#define lua_touint512(L, i) lua_touint512x(L, i, NULL)
#define lua_touint1024(L, i) lua_touint1024x(L, i, NULL)