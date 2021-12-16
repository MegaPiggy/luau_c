#include "lc.h"

#include "lua.h"
#include "lapi.h"
#include "lualib.h"
#include "lboost.h"

LUALIB_API int lc_register(lua_State* L, const char* libname, int size)
{
    if (libname)
    {
        /* check whether lib already exists */
        luaL_findtable(L, LUA_REGISTRYINDEX, "_LOADED", 1);
        lua_getfield(L, -1, libname); /* get _LOADED[libname] */
        if (!lua_istable(L, -1))
        {                  /* not found? */
            lua_pop(L, 1); /* remove previous result */
            /* try global variable (and create one if it does not exist) */
            if (luaL_findtable(L, LUA_GLOBALSINDEX, libname, size) != NULL)
                luaL_error(L, "name conflict for module '%s'", libname);
            lua_pushvalue(L, -1);
            lua_setfield(L, -3, libname); /* _LOADED[libname] = new table */
        }
        lua_remove(L, -2); /* remove _LOADED table */
    }
    return 1;
}

/* pushes new closure table onto the stack, using closure table at
 * given index as its parent */
LUALIB_API int lc_newclosuretable(lua_State * L, int idx)
{
    lua_newtable(L);
    lua_pushvalue(L,idx);
    lua_rawseti(L,-2,0);
    return 1;
}

/* gets upvalue with ID varid by consulting upvalue table at index
 * tidx for the upvalue table at given nesting level. */
LUALIB_API int lc_getupvalue(lua_State * L, int tidx, int level, int varid)
{
    if (level == 0)
        lua_rawgeti(L,tidx,varid);
    else
    {
        lua_pushvalue(L,tidx);
        while (--level >= 0)
        {
            lua_rawgeti(L,tidx,0); /* 0 links to parent table */
            lua_remove(L,-2);
            tidx = -1;
        }
        lua_rawgeti(L,-1,varid);
        lua_remove(L,-2);
    }
    return 1;
}


/* __add metamethod handler. */
LUALIB_API int lc_add(lua_State * L, int idxa, int idxb)
{
  if (lua_isnumber(L,idxa) && lua_isnumber(L,idxb))
    lua_pushnumber(L,lua_tonumber(L,idxa) + lua_tonumber(L,idxb));
  else
  {
    if (luaL_getmetafield(L,idxa,"__add")||luaL_getmetafield(L,idxb,"__add"))
    {
      lua_pushvalue(L,idxa < 0 && idxa > LUA_REGISTRYINDEX ? idxa-1 : idxa);
      lua_pushvalue(L,idxb < 0 && idxb > LUA_REGISTRYINDEX ? idxb-2 : idxb);
      lua_call(L,2,1);
    }
    else{
      luaL_error(L, "attempt to perform arithmetic");
      return 0;
    }
  }
  return 1;
}


/* __sub metamethod handler. */
LUALIB_API int lc_sub(lua_State * L, int idxa, int idxb)
{
  if (lua_isnumber(L,idxa) && lua_isnumber(L,idxb))
      lua_pushnumber(L,lua_tonumber(L,idxa) - lua_tonumber(L,idxb));
  else
  {
    if (luaL_getmetafield(L,idxa,"__sub")||luaL_getmetafield(L,idxb,"__sub"))
    {
      lua_pushvalue(L,idxa < 0 && idxa > LUA_REGISTRYINDEX ? idxa-1 : idxa);
      lua_pushvalue(L,idxb < 0 && idxb > LUA_REGISTRYINDEX ? idxb-2 : idxb);
      lua_call(L,2,1);
    }
    else{
      luaL_error(L, "attempt to perform arithmetic");
      return 0;
    }
  }
  return 1;
}


/* __mul metamethod handler. */
LUALIB_API int lc_mul(lua_State * L, int idxa, int idxb) {
  if (lua_isnumber(L,idxa) && lua_isnumber(L,idxb))
    lua_pushnumber(L,lua_tonumber(L,idxa) * lua_tonumber(L,idxb));
  else
  {
    if (luaL_getmetafield(L,idxa,"__mul")||luaL_getmetafield(L,idxb,"__mul"))
    {
      lua_pushvalue(L,idxa < 0 && idxa > LUA_REGISTRYINDEX ? idxa-1 : idxa);
      lua_pushvalue(L,idxb < 0 && idxb > LUA_REGISTRYINDEX ? idxb-2 : idxb);
      lua_call(L,2,1);
    }
    else{
      luaL_error(L, "attempt to perform arithmetic");
      return 0;
    }
  }
  return 1;
}


/* __div metamethod handler. */
LUALIB_API int lc_div(lua_State * L, int idxa, int idxb)
{
  if (lua_isnumber(L,idxa) && lua_isnumber(L,idxb))
    lua_pushnumber(L,lua_tonumber(L,idxa) / lua_tonumber(L,idxb));
  else
  {
    if (luaL_getmetafield(L,idxa,"__div")||luaL_getmetafield(L,idxb,"__div"))
    {
      lua_pushvalue(L,idxa < 0 && idxa > LUA_REGISTRYINDEX ? idxa-1 : idxa);
      lua_pushvalue(L,idxb < 0 && idxb > LUA_REGISTRYINDEX ? idxb-2 : idxb);
      lua_call(L,2,1);
    }
    else{
      luaL_error(L, "attempt to perform arithmetic");
      return 0;
    }
  }
  return 1;
}


/* __idiv metamethod handler. */
LUALIB_API int lc_idiv(lua_State * L, int idxa, int idxb)
{
  if (lua_isnumber(L,idxa) && lua_isnumber(L,idxb))
    lua_pushinteger(L,lua_tonumber(L,idxa) / lua_tonumber(L,idxb));
  else
  {
    if (luaL_getmetafield(L,idxa,"__idiv")||luaL_getmetafield(L,idxb,"__idiv"))
    {
      lua_pushvalue(L,idxa < 0 && idxa > LUA_REGISTRYINDEX ? idxa-1 : idxa);
      lua_pushvalue(L,idxb < 0 && idxb > LUA_REGISTRYINDEX ? idxb-2 : idxb);
      lua_call(L,2,1);
    }
    else{
      luaL_error(L, "attempt to perform arithmetic");
      return 0;
    }
  }
  return 1;
}


#include <math.h>

/* __mod metamethod handler. */
LUALIB_API int lc_mod(lua_State * L, int idxa, int idxb)
{
  if (lua_isnumber(L,idxa) && lua_isnumber(L,idxb))
    lua_pushnumber(L,lua_tonumber(L,idxa) - floor(lua_tonumber(L,idxa)/lua_tonumber(L,idxb))*lua_tonumber(L,idxb));
  else
  {
    if (luaL_getmetafield(L,idxa,"__mod")||luaL_getmetafield(L,idxb,"__mod"))
    {
      lua_pushvalue(L,idxa < 0 && idxa > LUA_REGISTRYINDEX ? idxa-1 : idxa);
      lua_pushvalue(L,idxb < 0 && idxb > LUA_REGISTRYINDEX ? idxb-2 : idxb);
      lua_call(L,2,1);
    }
    else{
      luaL_error(L, "attempt to perform arithmetic");
      return 0;
    }
  }
  return 1;
}


/* __le metamethod handler. */
LUALIB_API int lc_le(lua_State * L, int idxa, int idxb)
{
  if (lua_type(L,idxa) == LUA_TNUMBER && lua_type(L,idxb) == LUA_TNUMBER)
    return lua_tonumber(L,idxa) <= lua_tonumber(L,idxb);
  else if (lua_type(L,idxa) == LUA_TSTRING && lua_type(L,idxb) == LUA_TSTRING)
    /* result similar to lvm.c l_strcmp */
    return lua_lessthan(L,idxa,idxb) || lua_rawequal(L,idxa,idxb);
  else if (luaL_getmetafield(L,idxa,"__le")||luaL_getmetafield(L,idxb,"__le"))
  {
    lua_pushvalue(L,idxa < 0 && idxa > LUA_REGISTRYINDEX ? idxa-1 : idxa);
    lua_pushvalue(L,idxb < 0 && idxb > LUA_REGISTRYINDEX ? idxb-2 : idxb);
    lua_call(L,2,1);
    const int result = lua_toboolean(L,-1);
    lua_pop(L,1);
    return result;
  }
  else if (luaL_getmetafield(L,idxa,"__lt")||luaL_getmetafield(L,idxb,"__lt"))
  {
    lua_pushvalue(L,idxb < 0 && idxb > LUA_REGISTRYINDEX ? idxb-1 : idxb);
    lua_pushvalue(L,idxa < 0 && idxa > LUA_REGISTRYINDEX ? idxa-2 : idxa);
    lua_call(L,2,1);
    const int result = ! lua_toboolean(L,-1);
    lua_pop(L,1);
    return result;
  }
  else
    luaL_error(L, "attempt to compare");
  return 0;
}


/* __unm metamethod handler. */
LUALIB_API int lc_unm(lua_State * L, int idxa)
{
  if (lua_isnumber(L,idxa))
    lua_pushnumber(L,- lua_tonumber(L, idxa));
  else
  {
    if (luaL_getmetafield(L,idxa,"__unm"))
    {
      lua_pushvalue(L,idxa < 0 && idxa > LUA_REGISTRYINDEX ? idxa-1 : idxa);
      lua_call(L,1,1);
    }
    else{
      luaL_error(L, "attempt to perform arithmetic");
      return 0;
    }
  }
  return 1;
}