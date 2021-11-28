#pragma once

#include "lualib.h"

LUALIB_API int lc_register(lua_State* L, const char* libname, int size = 1);
LUALIB_API int lc_newclosuretable(lua_State* L, int idx);
LUALIB_API int lc_getupvalue(lua_State* L, int tidx, int level, int varid);
LUALIB_API int lc_add(lua_State* L, int idxa, int idxb);
LUALIB_API int lc_sub(lua_State* L, int idxa, int idxb);
LUALIB_API int lc_mul(lua_State* L, int idxa, int idxb);
LUALIB_API int lc_div(lua_State* L, int idxa, int idxb);
LUALIB_API int lc_mod(lua_State* L, int idxa, int idxb);
LUALIB_API int lc_le(lua_State* L, int idxa, int idxb);
LUALIB_API int lc_unm(lua_State* L, int idxa);