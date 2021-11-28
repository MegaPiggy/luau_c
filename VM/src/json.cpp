#include "lua.h"
#include "lapi.h"
#include "lualib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* pushes new closure table onto the stack, using closure table at
 * given index as its parent */
static void lc_newclosuretable(lua_State * L, int idx)
{
  lua_newtable(L);
  lua_pushvalue(L,idx);
  lua_rawseti(L,-2,0);
}

/* gets upvalue with ID varid by consulting upvalue table at index
 * tidx for the upvalue table at given nesting level. */
static void lc_getupvalue(lua_State * L, int tidx, int level, int varid)
{
  if (level == 0) {
    lua_rawgeti(L,tidx,varid);
  }
  else {
    lua_pushvalue(L,tidx);
    while (--level >= 0) {
      lua_rawgeti(L,tidx,0); /* 0 links to parent table */
      lua_remove(L,-2);
      tidx = -1;
    }
    lua_rawgeti(L,-1,varid);
    lua_remove(L,-2);
  }
}


/* name: escape_char
 * function(c) */
static int lcf1_escape_char (lua_State * L) {
  enum { lc_nformalargs = 1 };
  lua_settop(L,1);
  
  /* return "\\" .. (escape_char_map[c] or string.format("u%04x", c:byte())) */
  lua_pushliteral(L,"\\");
  lc_getupvalue(L,lua_upvalueindex(1),2,2);
  lua_pushvalue(L,1);
  lua_gettable(L,-2);
  lua_remove(L,-2);
  if (!(lua_toboolean(L,-1))) {
    lua_pop(L,1);
    lua_getfield(L,LUA_ENVIRONINDEX,"string");
    lua_pushliteral(L,"format");
    lua_gettable(L,-2);
    lua_remove(L,-2);
    const int lc6 = lua_gettop(L);
    lua_pushliteral(L,"u%04x");
    lua_pushvalue(L,1);
    lua_pushliteral(L,"byte");
    lua_gettable(L,-2);
    lua_insert(L,-2);
    lua_call(L,1,LUA_MULTRET);
    lua_call(L,(lua_gettop(L) - lc6),1);
  }
  lua_concat(L,2);
  return 1;
}


/* name: encode_nil
 * function(val) */
static int lcf1_encode_nil (lua_State * L) {
  enum { lc_nformalargs = 1 };
  lua_settop(L,1);
  
  /* return "null" */
  lua_pushliteral(L,"null");
  return 1;
}


/* __add metamethod handler. */
static void lc_add(lua_State * L, int idxa, int idxb) {
  if (lua_isnumber(L,idxa) && lua_isnumber(L,idxb)) {
    lua_pushnumber(L,lua_tonumber(L,idxa) + lua_tonumber(L,idxb));
  }
  else {
    if (luaL_getmetafield(L,idxa,"__add")||luaL_getmetafield(L,idxb,"__add")) {
      lua_pushvalue(L,idxa < 0 && idxa > LUA_REGISTRYINDEX ? idxa-1 : idxa);
      lua_pushvalue(L,idxb < 0 && idxb > LUA_REGISTRYINDEX ? idxb-2 : idxb);
      lua_call(L,2,1);
    }
    else {
      luaL_error(L, "attempt to perform arithmetic");
    }
  }
}


/* name: encode_table
 * function(val, stack) */
static int lcf1_encode_table (lua_State * L) {
  enum { lc_nformalargs = 2 };
  lua_settop(L,2);
  
  /* local res = {} */
  lua_newtable(L);
  
  /* stack = stack or {} */
  lua_pushvalue(L,2);
  if (!(lua_toboolean(L,-1))) {
    lua_pop(L,1);
    lua_newtable(L);
  }
  lua_replace(L,2);
  
  /* -- Circular reference?
   * if stack[val] then */
  enum { lc7 = 3 };
  lua_pushvalue(L,1);
  lua_gettable(L,2);
  const int lc8 = lua_toboolean(L,-1);
  lua_pop(L,1);
  if (lc8) {
    
    /* error("circular reference") */
    lua_getfield(L,LUA_ENVIRONINDEX,"error");
    lua_pushliteral(L,"circular reference");
    lua_call(L,1,0);
  }
  lua_settop(L,lc7);
  
  /* stack[val] = true */
  lua_pushboolean(L,1);
  lua_pushvalue(L,1);
  lua_insert(L,-2);
  lua_settable(L,2);
  
  /* if rawget(val, 1) ~= nil or next(val) == nil then */
  enum { lc9 = 3 };
  lua_getfield(L,LUA_ENVIRONINDEX,"rawget");
  lua_pushvalue(L,1);
  lua_pushnumber(L,1);
  lua_call(L,2,1);
  lua_pushnil(L);
  const int lc10 = lua_equal(L,-2,-1);
  lua_pop(L,2);
  lua_pushboolean(L,lc10);
  lua_pushboolean(L,!(lua_toboolean(L,-1)));
  lua_remove(L,-2);
  if (!(lua_toboolean(L,-1))) {
    lua_pop(L,1);
    lua_getfield(L,LUA_ENVIRONINDEX,"next");
    lua_pushvalue(L,1);
    lua_call(L,1,1);
    lua_pushnil(L);
    const int lc11 = lua_equal(L,-2,-1);
    lua_pop(L,2);
    lua_pushboolean(L,lc11);
  }
  const int lc12 = lua_toboolean(L,-1);
  lua_pop(L,1);
  if (lc12) {
    
    /* -- Treat as array -- check keys are valid and it is not sparse
     * local n = 0 */
    lua_pushnumber(L,0);
    
    /* for k in pairs(val) do
     * internal: local f, s, var = explist */
    enum { lc13 = 4 };
    lua_getfield(L,LUA_ENVIRONINDEX,"pairs");
    lua_pushvalue(L,1);
    lua_call(L,1,3);
    while (1) {
      
      /* internal: local var_1, ..., var_n = f(s, var)
       *           if var_1 == nil then break end
       *           var = var_1 */
      lua_pushvalue(L,-3);
      lua_pushvalue(L,-3);
      lua_pushvalue(L,-3);
      lua_call(L,2,1);
      if (lua_isnil(L,-1)) {
        break;
      }
      lua_pushvalue(L,-1);
      lua_replace(L,-3);
      
      /* internal: local k with idx 8 */
      
      
      /* if type(k) ~= "number" then */
      enum { lc14 = 8 };
      lua_getfield(L,LUA_ENVIRONINDEX,"type");
      lua_pushvalue(L,8);
      lua_call(L,1,1);
      lua_pushliteral(L,"number");
      const int lc15 = lua_equal(L,-2,-1);
      lua_pop(L,2);
      lua_pushboolean(L,lc15);
      lua_pushboolean(L,!(lua_toboolean(L,-1)));
      lua_remove(L,-2);
      const int lc16 = lua_toboolean(L,-1);
      lua_pop(L,1);
      if (lc16) {
        
        /* error("invalid table: mixed or invalid key types") */
        lua_getfield(L,LUA_ENVIRONINDEX,"error");
        lua_pushliteral(L,"invalid table: mixed or invalid key types");
        lua_call(L,1,0);
      }
      lua_settop(L,lc14);
      
      /* n = n + 1 */
      lua_pushnumber(L,1);
      lc_add(L,4,-1);
      lua_remove(L,-2);
      lua_replace(L,4);
      
      /* internal: stack cleanup on scope exit */
      lua_pop(L,1);
    }
    lua_settop(L,lc13);
    
    /* if n ~= #val then */
    enum { lc17 = 4 };
    const double lc18 = lua_objlen(L,1);
    lua_pushnumber(L,lc18);
    const int lc19 = lua_equal(L,4,-1);
    lua_pop(L,1);
    lua_pushboolean(L,lc19);
    lua_pushboolean(L,!(lua_toboolean(L,-1)));
    lua_remove(L,-2);
    const int lc20 = lua_toboolean(L,-1);
    lua_pop(L,1);
    if (lc20) {
      
      /* error("invalid table: sparse array") */
      lua_getfield(L,LUA_ENVIRONINDEX,"error");
      lua_pushliteral(L,"invalid table: sparse array");
      lua_call(L,1,0);
    }
    lua_settop(L,lc17);
    
    /* -- Encode
     * for i, v in ipairs(val) do
     * internal: local f, s, var = explist */
    enum { lc21 = 4 };
    lua_getfield(L,LUA_ENVIRONINDEX,"ipairs");
    lua_pushvalue(L,1);
    lua_call(L,1,3);
    while (1) {
      
      /* internal: local var_1, ..., var_n = f(s, var)
       *           if var_1 == nil then break end
       *           var = var_1 */
      lua_pushvalue(L,-3);
      lua_pushvalue(L,-3);
      lua_pushvalue(L,-3);
      lua_call(L,2,2);
      if (lua_isnil(L,-2)) {
        break;
      }
      lua_pushvalue(L,-2);
      lua_replace(L,-4);
      
      /* internal: local i with idx 8
       * internal: local v with idx 9 */
      
      
      /* table.insert(res, encode(v, stack)) */
      lua_getfield(L,LUA_ENVIRONINDEX,"table");
      lua_pushliteral(L,"insert");
      lua_gettable(L,-2);
      lua_remove(L,-2);
      const int lc22 = lua_gettop(L);
      lua_pushvalue(L,3);
      lc_getupvalue(L,lua_upvalueindex(1),3,1);
      lua_pushvalue(L,9);
      lua_pushvalue(L,2);
      lua_call(L,2,LUA_MULTRET);
      lua_call(L,(lua_gettop(L) - lc22),0);
      
      /* internal: stack cleanup on scope exit */
      lua_pop(L,2);
    }
    lua_settop(L,lc21);
    
    /* stack[val] = nil */
    lua_pushnil(L);
    lua_pushvalue(L,1);
    lua_insert(L,-2);
    lua_settable(L,2);
    
    /* return "[" .. table.concat(res, ",") .. "]" */
    lua_pushliteral(L,"[");
    lua_getfield(L,LUA_ENVIRONINDEX,"table");
    lua_pushliteral(L,"concat");
    lua_gettable(L,-2);
    lua_remove(L,-2);
    lua_pushvalue(L,3);
    lua_pushliteral(L,",");
    lua_call(L,2,1);
    lua_pushliteral(L,"]");
    lua_concat(L,2);
    lua_concat(L,2);
    return 1;
  }
  else {
    
    /* else
     * -- Treat as an object
     * for k, v in pairs(val) do
     * internal: local f, s, var = explist */
    enum { lc23 = 3 };
    lua_getfield(L,LUA_ENVIRONINDEX,"pairs");
    lua_pushvalue(L,1);
    lua_call(L,1,3);
    while (1) {
      
      /* internal: local var_1, ..., var_n = f(s, var)
       *           if var_1 == nil then break end
       *           var = var_1 */
      lua_pushvalue(L,-3);
      lua_pushvalue(L,-3);
      lua_pushvalue(L,-3);
      lua_call(L,2,2);
      if (lua_isnil(L,-2)) {
        break;
      }
      lua_pushvalue(L,-2);
      lua_replace(L,-4);
      
      /* internal: local k with idx 7
       * internal: local v with idx 8 */
      
      
      /* if type(k) ~= "string" then */
      enum { lc24 = 8 };
      lua_getfield(L,LUA_ENVIRONINDEX,"type");
      lua_pushvalue(L,7);
      lua_call(L,1,1);
      lua_pushliteral(L,"string");
      const int lc25 = lua_equal(L,-2,-1);
      lua_pop(L,2);
      lua_pushboolean(L,lc25);
      lua_pushboolean(L,!(lua_toboolean(L,-1)));
      lua_remove(L,-2);
      const int lc26 = lua_toboolean(L,-1);
      lua_pop(L,1);
      if (lc26) {
        
        /* error("invalid table: mixed or invalid key types") */
        lua_getfield(L,LUA_ENVIRONINDEX,"error");
        lua_pushliteral(L,"invalid table: mixed or invalid key types");
        lua_call(L,1,0);
      }
      lua_settop(L,lc24);
      
      /* table.insert(res, encode(k, stack) .. ":" .. encode(v, stack)) */
      lua_getfield(L,LUA_ENVIRONINDEX,"table");
      lua_pushliteral(L,"insert");
      lua_gettable(L,-2);
      lua_remove(L,-2);
      lua_pushvalue(L,3);
      lc_getupvalue(L,lua_upvalueindex(1),3,1);
      lua_pushvalue(L,7);
      lua_pushvalue(L,2);
      lua_call(L,2,1);
      lua_pushliteral(L,":");
      lc_getupvalue(L,lua_upvalueindex(1),3,1);
      lua_pushvalue(L,8);
      lua_pushvalue(L,2);
      lua_call(L,2,1);
      lua_concat(L,2);
      lua_concat(L,2);
      lua_call(L,2,0);
      
      /* internal: stack cleanup on scope exit */
      lua_pop(L,2);
    }
    lua_settop(L,lc23);
    
    /* stack[val] = nil */
    lua_pushnil(L);
    lua_pushvalue(L,1);
    lua_insert(L,-2);
    lua_settable(L,2);
    
    /* return "{" .. table.concat(res, ",") .. "}" */
    lua_pushliteral(L,"{");
    lua_getfield(L,LUA_ENVIRONINDEX,"table");
    lua_pushliteral(L,"concat");
    lua_gettable(L,-2);
    lua_remove(L,-2);
    lua_pushvalue(L,3);
    lua_pushliteral(L,",");
    lua_call(L,2,1);
    lua_pushliteral(L,"}");
    lua_concat(L,2);
    lua_concat(L,2);
    return 1;
  }
  lua_settop(L,lc9);
  return 0;
}


/* name: encode_string
 * function(val) */
static int lcf1_encode_string (lua_State * L) {
  enum { lc_nformalargs = 1 };
  lua_settop(L,1);
  
  /* return '"' .. val:gsub('[%z\1-\31\\"]', escape_char) .. '"' */
  lua_pushliteral(L,"\"");
  lua_pushvalue(L,1);
  lua_pushliteral(L,"gsub");
  lua_gettable(L,-2);
  lua_insert(L,-2);
  lua_pushliteral(L,"[%z-\\\"]");
  lc_getupvalue(L,lua_upvalueindex(1),0,4);
  lua_call(L,3,1);
  lua_pushliteral(L,"\"");
  lua_concat(L,2);
  lua_concat(L,2);
  return 1;
}


/* __le metamethod handler. */
static int lc_le(lua_State * L, int idxa, int idxb) {
  if (lua_type(L,idxa) == LUA_TNUMBER && lua_type(L,idxb) == LUA_TNUMBER) {
    return lua_tonumber(L,idxa) <= lua_tonumber(L,idxb);
  }
  else if (lua_type(L,idxa) == LUA_TSTRING && lua_type(L,idxb) == LUA_TSTRING) {
    /* result similar to lvm.c l_strcmp */
    return lua_lessthan(L,idxa,idxb) || lua_rawequal(L,idxa,idxb);
  }
  else if (luaL_getmetafield(L,idxa,"__le")||luaL_getmetafield(L,idxb,"__le")) {
    lua_pushvalue(L,idxa < 0 && idxa > LUA_REGISTRYINDEX ? idxa-1 : idxa);
    lua_pushvalue(L,idxb < 0 && idxb > LUA_REGISTRYINDEX ? idxb-2 : idxb);
    lua_call(L,2,1);
    const int result = lua_toboolean(L,-1);
    lua_pop(L,1);
    return result;
  }
  else if (luaL_getmetafield(L,idxa,"__lt")||luaL_getmetafield(L,idxb,"__lt")) {
    lua_pushvalue(L,idxb < 0 && idxb > LUA_REGISTRYINDEX ? idxb-1 : idxb);
    lua_pushvalue(L,idxa < 0 && idxa > LUA_REGISTRYINDEX ? idxa-2 : idxa);
    lua_call(L,2,1);
    const int result = ! lua_toboolean(L,-1);
    lua_pop(L,1);
    return result;
  }
  else {
    luaL_error(L, "attempt to compare");
  }
}


/* __unm metamethod handler. */
static void lc_unm(lua_State * L, int idxa) {
  if (lua_isnumber(L,idxa)) {
    lua_pushnumber(L,- lua_tonumber(L, idxa));
  }
  else {
    if (luaL_getmetafield(L,idxa,"__unm")) {
      lua_pushvalue(L,idxa < 0 && idxa > LUA_REGISTRYINDEX ? idxa-1 : idxa);
      lua_call(L,1,1);
    }
    else {
      luaL_error(L, "attempt to perform arithmetic");
    }
  }
}


/* name: encode_number
 * function(val) */
static int lcf1_encode_number (lua_State * L) {
  lua_settop(L,1);
  
  /* -- Check for NaN, -inf and inf
   * if val ~= val or val <= -math.huge or val >= math.huge then */
  enum { lc27 = 1 };
  const int lc28 = lua_equal(L,1,1);
  lua_pushboolean(L,lc28);
  lua_pushboolean(L,!(lua_toboolean(L,-1)));
  lua_remove(L,-2);
  if (!(lua_toboolean(L,-1))) {
    lua_pop(L,1);
    lua_getfield(L,LUA_ENVIRONINDEX,"math");
    lua_pushliteral(L,"huge");
    lua_gettable(L,-2);
    lua_remove(L,-2);
    lc_unm(L,-1);
    lua_remove(L,-2);
    const int lc29 = lc_le(L,1,-1);
    lua_pop(L,1);
    lua_pushboolean(L,lc29);
  }
  if (!(lua_toboolean(L,-1))) {
    lua_pop(L,1);
    lua_getfield(L,LUA_ENVIRONINDEX,"math");
    lua_pushliteral(L,"huge");
    lua_gettable(L,-2);
    lua_remove(L,-2);
    const int lc30 = lc_le(L,-1,1);
    lua_pop(L,1);
    lua_pushboolean(L,lc30);
  }
  const int lc31 = lua_toboolean(L,-1);
  lua_pop(L,1);
  if (lc31) {
    
    /* error("unexpected number value '" .. tostring(val) .. "'") */
    lua_getfield(L,LUA_ENVIRONINDEX,"error");
    lua_pushliteral(L,"unexpected number value '");
    lua_getfield(L,LUA_ENVIRONINDEX,"tostring");
    lua_pushvalue(L,1);
    lua_call(L,1,1);
    lua_pushliteral(L,"'");
    lua_concat(L,2);
    lua_concat(L,2);
    lua_call(L,1,0);
  }
  lua_settop(L,lc27);
  
  /* return string.format("%.14g", val) */
  const int lc32 = lua_gettop(L);
  lua_getfield(L,LUA_ENVIRONINDEX,"string");
  lua_pushliteral(L,"format");
  lua_gettable(L,-2);
  lua_remove(L,-2);
  lua_pushliteral(L,"%.14g");
  lua_pushvalue(L,1);
  lua_call(L,2,LUA_MULTRET);
  return (lua_gettop(L) - lc32);
}


/* name: encode
 * function(val, stack) */
static int lcf1_encode (lua_State * L) {
  lua_settop(L,2);
  
  /* local t = type(val) */
  lua_getfield(L,LUA_ENVIRONINDEX,"type");
  lua_pushvalue(L,1);
  lua_call(L,1,1);
  
  /* local f = type_func_map[t] */
  lc_getupvalue(L,lua_upvalueindex(1),0,5);
  lua_pushvalue(L,3);
  lua_gettable(L,-2);
  lua_remove(L,-2);
  
  /* if f then */
  enum { lc34 = 4 };
  if (lua_toboolean(L,4)) {
    
    /* return f(val, stack) */
    const int lc35 = lua_gettop(L);
    lua_pushvalue(L,4);
    lua_pushvalue(L,1);
    lua_pushvalue(L,2);
    lua_call(L,2,LUA_MULTRET);
    return (lua_gettop(L) - lc35);
  }
  lua_settop(L,lc34);
  
  /* error("unexpected type '" .. t .. "'") */
  lua_getfield(L,LUA_ENVIRONINDEX,"error");
  lua_pushliteral(L,"unexpected type '");
  lua_pushvalue(L,3);
  lua_pushliteral(L,"'");
  lua_concat(L,2);
  lua_concat(L,2);
  lua_call(L,1,0);
  return 0;
}


static void lc_setupvalue(lua_State * L, int tidx, int level, int varid) {
  if (level == 0) {
    lua_rawseti(L,tidx,varid);
  }
  else {
    lua_pushvalue(L,tidx);
    while(--level >= 0) {
      lua_rawgeti(L,tidx,0); /* 0 links to parent table */
      lua_remove(L,-2);
      tidx = -1;
    }
    lua_insert(L,-2);
    lua_rawseti(L,-2,varid);
    lua_pop(L,1);
  }
}


/* name: json.encode
 * function(val) */
static int lcf1_json_encode (lua_State * L) {
  lua_settop(L,1);
  
  /* return ( encode(val) ) */
  lc_getupvalue(L,lua_upvalueindex(1),4,1);
  lua_pushvalue(L,1);
  lua_call(L,1,1);
  return 1;
}


/* name: create_set
 * function(...) */
static int lcf1_create_set (lua_State * L) {
  enum { lc_nformalargs = 0 };
  const int lc_nactualargs = lua_gettop(L);
  const int lc_nextra = (lc_nactualargs - lc_nformalargs);
  
  /* local res = {} */
  lua_newtable(L);
  
  /* for i = 1, select("#", ...) do */
  lua_pushnumber(L,1);
  lua_getfield(L,LUA_ENVIRONINDEX,"select");
  const int lc40 = lua_gettop(L);
  lua_pushliteral(L,"#");
  {int i; for (i=lc_nformalargs+1; i<=lc_nactualargs; i++) { lua_pushvalue(L, i); }}
  lua_call(L,(lua_gettop(L) - lc40),1);
  if (!((lua_isnumber(L,-2) && lua_isnumber(L,-1)))) {
    luaL_error(L,"'for' limit must be a number");
  }
  double lc37_var = lua_tonumber(L,-2);
  const double lc38_limit = lua_tonumber(L,-1);
  const double lc39_step = 1;
  lua_pop(L,2);
  enum { lc41 = 1 };
  while ((((lc39_step > 0) && (lc37_var <= lc38_limit)) || ((lc39_step <= 0) && (lc37_var >= lc38_limit)))) {
    
    /* internal: local i at index 2 */
    lua_pushnumber(L,lc37_var);
    
    /* res[ select(i, ...) ] = true */
    lua_pushboolean(L,1);
    lua_getfield(L,LUA_ENVIRONINDEX,"select");
    const int lc42 = lua_gettop(L);
    lua_pushvalue(L,(2 + lc_nextra));
    {int i; for (i=lc_nformalargs+1; i<=lc_nactualargs; i++) { lua_pushvalue(L, i); }}
    lua_call(L,(lua_gettop(L) - lc42),1);
    lua_insert(L,-2);
    lua_settable(L,(1 + lc_nextra));
    
    /* internal: stack cleanup on scope exit */
    lua_pop(L,1);
    lc37_var += lc39_step;
  }
  lua_settop(L,(lc41 + lc_nextra));
  
  /* return res */
  lua_pushvalue(L,(1 + lc_nextra));
  return 1;
}


/* name: next_char
 * function(str, idx, set, negate) */
static int lcf1_next_char (lua_State * L) {
  enum { lc_nformalargs = 4 };
  lua_settop(L,4);
  
  /* for i = idx, #str do */
  const double lc52 = lua_objlen(L,1);
  lua_pushnumber(L,lc52);
  if (!((lua_isnumber(L,2) && lua_isnumber(L,-1)))) {
    luaL_error(L,"'for' limit must be a number");
  }
  double lc49_var = lua_tonumber(L,2);
  const double lc50_limit = lua_tonumber(L,-1);
  const double lc51_step = 1;
  lua_pop(L,1);
  enum { lc53 = 4 };
  while ((((lc51_step > 0) && (lc49_var <= lc50_limit)) || ((lc51_step <= 0) && (lc49_var >= lc50_limit)))) {
    
    /* internal: local i at index 5 */
    lua_pushnumber(L,lc49_var);
    
    /* if set[str:sub(i, i)] ~= negate then */
    enum { lc54 = 5 };
    lua_pushvalue(L,1);
    lua_pushliteral(L,"sub");
    lua_gettable(L,-2);
    lua_insert(L,-2);
    lua_pushvalue(L,5);
    lua_pushvalue(L,5);
    lua_call(L,3,1);
    lua_gettable(L,3);
    const int lc55 = lua_equal(L,-1,4);
    lua_pop(L,1);
    lua_pushboolean(L,lc55);
    lua_pushboolean(L,!(lua_toboolean(L,-1)));
    lua_remove(L,-2);
    const int lc56 = lua_toboolean(L,-1);
    lua_pop(L,1);
    if (lc56) {
      
      /* return i */
      lua_pushvalue(L,5);
      return 1;
    }
    lua_settop(L,lc54);
    
    /* internal: stack cleanup on scope exit */
    lua_pop(L,1);
    lc49_var += lc51_step;
  }
  lua_settop(L,lc53);
  
  /* return #str + 1 */
  const double lc57 = lua_objlen(L,1);
  lua_pushnumber(L,lc57);
  lua_pushnumber(L,1);
  lc_add(L,-2,-1);
  lua_remove(L,-2);
  lua_remove(L,-2);
  return 1;
}


/* __sub metamethod handler. */
static void lc_sub(lua_State * L, int idxa, int idxb) {
  if (lua_isnumber(L,idxa) && lua_isnumber(L,idxb)) {
    lua_pushnumber(L,lua_tonumber(L,idxa) - lua_tonumber(L,idxb));
  }
  else {
    if (luaL_getmetafield(L,idxa,"__sub")||luaL_getmetafield(L,idxb,"__sub")) {
      lua_pushvalue(L,idxa < 0 && idxa > LUA_REGISTRYINDEX ? idxa-1 : idxa);
      lua_pushvalue(L,idxb < 0 && idxb > LUA_REGISTRYINDEX ? idxb-2 : idxb);
      lua_call(L,2,1);
    }
    else {
      luaL_error(L, "attempt to perform arithmetic");
    }
  }
}


/* name: decode_error
 * function(str, idx, msg) */
static int lcf1_decode_error (lua_State * L) {
  enum { lc_nformalargs = 3 };
  lua_settop(L,3);
  
  /* local line_count = 1 */
  lua_pushnumber(L,1);
  
  /* local col_count = 1 */
  lua_pushnumber(L,1);
  
  /* for i = 1, idx - 1 do */
  lua_pushnumber(L,1);
  lua_pushnumber(L,1);
  lc_sub(L,2,-1);
  lua_remove(L,-2);
  if (!((lua_isnumber(L,-2) && lua_isnumber(L,-1)))) {
    luaL_error(L,"'for' limit must be a number");
  }
  double lc59_var = lua_tonumber(L,-2);
  const double lc60_limit = lua_tonumber(L,-1);
  const double lc61_step = 1;
  lua_pop(L,2);
  enum { lc62 = 5 };
  while ((((lc61_step > 0) && (lc59_var <= lc60_limit)) || ((lc61_step <= 0) && (lc59_var >= lc60_limit)))) {
    
    /* internal: local i at index 6 */
    lua_pushnumber(L,lc59_var);
    
    /* col_count = col_count + 1 */
    lua_pushnumber(L,1);
    lc_add(L,5,-1);
    lua_remove(L,-2);
    lua_replace(L,5);
    
    /* if str:sub(i, i) == "\n" then */
    enum { lc63 = 6 };
    lua_pushvalue(L,1);
    lua_pushliteral(L,"sub");
    lua_gettable(L,-2);
    lua_insert(L,-2);
    lua_pushvalue(L,6);
    lua_pushvalue(L,6);
    lua_call(L,3,1);
    lua_pushliteral(L,"\n");
    const int lc64 = lua_equal(L,-2,-1);
    lua_pop(L,2);
    lua_pushboolean(L,lc64);
    const int lc65 = lua_toboolean(L,-1);
    lua_pop(L,1);
    if (lc65) {
      
      /* line_count = line_count + 1 */
      lua_pushnumber(L,1);
      lc_add(L,4,-1);
      lua_remove(L,-2);
      lua_replace(L,4);
      
      /* col_count = 1 */
      lua_pushnumber(L,1);
      lua_replace(L,5);
    }
    lua_settop(L,lc63);
    
    /* internal: stack cleanup on scope exit */
    lua_pop(L,1);
    lc59_var += lc61_step;
  }
  lua_settop(L,lc62);
  
  /* error( string.format("%s at line %d col %d", msg, line_count, col_count) ) */
  lua_getfield(L,LUA_ENVIRONINDEX,"error");
  const int lc66 = lua_gettop(L);
  lua_getfield(L,LUA_ENVIRONINDEX,"string");
  lua_pushliteral(L,"format");
  lua_gettable(L,-2);
  lua_remove(L,-2);
  lua_pushliteral(L,"%s at line %d col %d");
  lua_pushvalue(L,3);
  lua_pushvalue(L,4);
  lua_pushvalue(L,5);
  lua_call(L,4,LUA_MULTRET);
  lua_call(L,(lua_gettop(L) - lc66),0);
  return 0;
}


/* __div metamethod handler. */
static void lc_div(lua_State * L, int idxa, int idxb) {
  if (lua_isnumber(L,idxa) && lua_isnumber(L,idxb)) {
    lua_pushnumber(L,lua_tonumber(L,idxa) / lua_tonumber(L,idxb));
  }
  else {
    if (luaL_getmetafield(L,idxa,"__div")||luaL_getmetafield(L,idxb,"__div")) {
      lua_pushvalue(L,idxa < 0 && idxa > LUA_REGISTRYINDEX ? idxa-1 : idxa);
      lua_pushvalue(L,idxb < 0 && idxb > LUA_REGISTRYINDEX ? idxb-2 : idxb);
      lua_call(L,2,1);
    }
    else {
      luaL_error(L, "attempt to perform arithmetic");
    }
  }
}


#include <math.h>

/* __mod metamethod handler. */
static void lc_mod(lua_State * L, int idxa, int idxb) {
  if (lua_isnumber(L,idxa) && lua_isnumber(L,idxb)) {
    lua_pushnumber(L,lua_tonumber(L,idxa) - floor(lua_tonumber(L,idxa)/lua_tonumber(L,idxb))*lua_tonumber(L,idxb));
  }
  else {
    if (luaL_getmetafield(L,idxa,"__mod")||luaL_getmetafield(L,idxb,"__mod")) {
      lua_pushvalue(L,idxa < 0 && idxa > LUA_REGISTRYINDEX ? idxa-1 : idxa);
      lua_pushvalue(L,idxb < 0 && idxb > LUA_REGISTRYINDEX ? idxb-2 : idxb);
      lua_call(L,2,1);
    }
    else {
      luaL_error(L, "attempt to perform arithmetic");
    }
  }
}


/* name: codepoint_to_utf8
 * function(n) */
static int lcf1_codepoint_to_utf8 (lua_State * L) {
  enum { lc_nformalargs = 1 };
  lua_settop(L,1);
  
  /* -- http://scripts.sil.org/cms/scripts/page.php?site_id=nrsi&id=iws-appendixa
   * local f = math.floor */
  lua_getfield(L,LUA_ENVIRONINDEX,"math");
  lua_pushliteral(L,"floor");
  lua_gettable(L,-2);
  lua_remove(L,-2);
  
  /* if n <= 0x7f then */
  enum { lc68 = 2 };
  lua_pushnumber(L,127);
  const int lc69 = lc_le(L,1,-1);
  lua_pop(L,1);
  lua_pushboolean(L,lc69);
  const int lc70 = lua_toboolean(L,-1);
  lua_pop(L,1);
  if (lc70) {
    
    /* return string.char(n) */
    const int lc71 = lua_gettop(L);
    lua_getfield(L,LUA_ENVIRONINDEX,"string");
    lua_pushliteral(L,"char");
    lua_gettable(L,-2);
    lua_remove(L,-2);
    lua_pushvalue(L,1);
    lua_call(L,1,LUA_MULTRET);
    return (lua_gettop(L) - lc71);
  }
  else {
    
    /* elseif n <= 0x7ff then */
    enum { lc72 = 2 };
    lua_pushnumber(L,2047);
    const int lc73 = lc_le(L,1,-1);
    lua_pop(L,1);
    lua_pushboolean(L,lc73);
    const int lc74 = lua_toboolean(L,-1);
    lua_pop(L,1);
    if (lc74) {
      
      /* return string.char(f(n / 64) + 192, n % 64 + 128) */
      const int lc75 = lua_gettop(L);
      lua_getfield(L,LUA_ENVIRONINDEX,"string");
      lua_pushliteral(L,"char");
      lua_gettable(L,-2);
      lua_remove(L,-2);
      lua_pushvalue(L,2);
      lua_pushnumber(L,64);
      lc_div(L,1,-1);
      lua_remove(L,-2);
      lua_call(L,1,1);
      lua_pushnumber(L,192);
      lc_add(L,-2,-1);
      lua_remove(L,-2);
      lua_remove(L,-2);
      lua_pushnumber(L,64);
      lc_mod(L,1,-1);
      lua_remove(L,-2);
      lua_pushnumber(L,128);
      lc_add(L,-2,-1);
      lua_remove(L,-2);
      lua_remove(L,-2);
      lua_call(L,2,LUA_MULTRET);
      return (lua_gettop(L) - lc75);
    }
    else {
      
      /* elseif n <= 0xffff then */
      enum { lc76 = 2 };
      lua_pushnumber(L,65535);
      const int lc77 = lc_le(L,1,-1);
      lua_pop(L,1);
      lua_pushboolean(L,lc77);
      const int lc78 = lua_toboolean(L,-1);
      lua_pop(L,1);
      if (lc78) {
        
        /* return string.char(f(n / 4096) + 224, f(n % 4096 / 64) + 128, n % 64 + 128) */
        const int lc79 = lua_gettop(L);
        lua_getfield(L,LUA_ENVIRONINDEX,"string");
        lua_pushliteral(L,"char");
        lua_gettable(L,-2);
        lua_remove(L,-2);
        lua_pushvalue(L,2);
        lua_pushnumber(L,4096);
        lc_div(L,1,-1);
        lua_remove(L,-2);
        lua_call(L,1,1);
        lua_pushnumber(L,224);
        lc_add(L,-2,-1);
        lua_remove(L,-2);
        lua_remove(L,-2);
        lua_pushvalue(L,2);
        lua_pushnumber(L,4096);
        lc_mod(L,1,-1);
        lua_remove(L,-2);
        lua_pushnumber(L,64);
        lc_div(L,-2,-1);
        lua_remove(L,-2);
        lua_remove(L,-2);
        lua_call(L,1,1);
        lua_pushnumber(L,128);
        lc_add(L,-2,-1);
        lua_remove(L,-2);
        lua_remove(L,-2);
        lua_pushnumber(L,64);
        lc_mod(L,1,-1);
        lua_remove(L,-2);
        lua_pushnumber(L,128);
        lc_add(L,-2,-1);
        lua_remove(L,-2);
        lua_remove(L,-2);
        lua_call(L,3,LUA_MULTRET);
        return (lua_gettop(L) - lc79);
      }
      else {
        
        /* elseif n <= 0x10ffff then */
        enum { lc80 = 2 };
        lua_pushnumber(L,1114111);
        const int lc81 = lc_le(L,1,-1);
        lua_pop(L,1);
        lua_pushboolean(L,lc81);
        const int lc82 = lua_toboolean(L,-1);
        lua_pop(L,1);
        if (lc82) {
          
          /* return string.char(f(n / 262144) + 240, f(n % 262144 / 4096) + 128,
           *                        f(n % 4096 / 64) + 128, n % 64 + 128) */
          const int lc83 = lua_gettop(L);
          lua_getfield(L,LUA_ENVIRONINDEX,"string");
          lua_pushliteral(L,"char");
          lua_gettable(L,-2);
          lua_remove(L,-2);
          lua_pushvalue(L,2);
          lua_pushnumber(L,262144);
          lc_div(L,1,-1);
          lua_remove(L,-2);
          lua_call(L,1,1);
          lua_pushnumber(L,240);
          lc_add(L,-2,-1);
          lua_remove(L,-2);
          lua_remove(L,-2);
          lua_pushvalue(L,2);
          lua_pushnumber(L,262144);
          lc_mod(L,1,-1);
          lua_remove(L,-2);
          lua_pushnumber(L,4096);
          lc_div(L,-2,-1);
          lua_remove(L,-2);
          lua_remove(L,-2);
          lua_call(L,1,1);
          lua_pushnumber(L,128);
          lc_add(L,-2,-1);
          lua_remove(L,-2);
          lua_remove(L,-2);
          lua_pushvalue(L,2);
          lua_pushnumber(L,4096);
          lc_mod(L,1,-1);
          lua_remove(L,-2);
          lua_pushnumber(L,64);
          lc_div(L,-2,-1);
          lua_remove(L,-2);
          lua_remove(L,-2);
          lua_call(L,1,1);
          lua_pushnumber(L,128);
          lc_add(L,-2,-1);
          lua_remove(L,-2);
          lua_remove(L,-2);
          lua_pushnumber(L,64);
          lc_mod(L,1,-1);
          lua_remove(L,-2);
          lua_pushnumber(L,128);
          lc_add(L,-2,-1);
          lua_remove(L,-2);
          lua_remove(L,-2);
          lua_call(L,4,LUA_MULTRET);
          return (lua_gettop(L) - lc83);
        }
        lua_settop(L,lc80);
      }
      lua_settop(L,lc76);
    }
    lua_settop(L,lc72);
  }
  lua_settop(L,lc68);
  
  /* error( string.format("invalid unicode codepoint '%x'", n) ) */
  lua_getfield(L,LUA_ENVIRONINDEX,"error");
  const int lc84 = lua_gettop(L);
  lua_getfield(L,LUA_ENVIRONINDEX,"string");
  lua_pushliteral(L,"format");
  lua_gettable(L,-2);
  lua_remove(L,-2);
  lua_pushliteral(L,"invalid unicode codepoint '%x'");
  lua_pushvalue(L,1);
  lua_call(L,2,LUA_MULTRET);
  lua_call(L,(lua_gettop(L) - lc84),0);
  return 0;
}


/* __mul metamethod handler. */
static void lc_mul(lua_State * L, int idxa, int idxb) {
  if (lua_isnumber(L,idxa) && lua_isnumber(L,idxb)) {
    lua_pushnumber(L,lua_tonumber(L,idxa) * lua_tonumber(L,idxb));
  }
  else {
    if (luaL_getmetafield(L,idxa,"__mul")||luaL_getmetafield(L,idxb,"__mul")) {
      lua_pushvalue(L,idxa < 0 && idxa > LUA_REGISTRYINDEX ? idxa-1 : idxa);
      lua_pushvalue(L,idxb < 0 && idxb > LUA_REGISTRYINDEX ? idxb-2 : idxb);
      lua_call(L,2,1);
    }
    else {
      luaL_error(L, "attempt to perform arithmetic");
    }
  }
}


/* name: parse_unicode_escape
 * function(s) */
static int lcf1_parse_unicode_escape (lua_State * L) {
  enum { lc_nformalargs = 1 };
  lua_settop(L,1);
  
  /* local n1 = tonumber( s:sub(1, 4),  16 ) */
  lua_getfield(L,LUA_ENVIRONINDEX,"tonumber");
  lua_pushvalue(L,1);
  lua_pushliteral(L,"sub");
  lua_gettable(L,-2);
  lua_insert(L,-2);
  lua_pushnumber(L,1);
  lua_pushnumber(L,4);
  lua_call(L,3,1);
  lua_pushnumber(L,16);
  lua_call(L,2,1);
  
  /* local n2 = tonumber( s:sub(7, 10), 16 ) */
  lua_getfield(L,LUA_ENVIRONINDEX,"tonumber");
  lua_pushvalue(L,1);
  lua_pushliteral(L,"sub");
  lua_gettable(L,-2);
  lua_insert(L,-2);
  lua_pushnumber(L,7);
  lua_pushnumber(L,10);
  lua_call(L,3,1);
  lua_pushnumber(L,16);
  lua_call(L,2,1);
  
  /* -- Surrogate pair?
   * if n2 then */
  enum { lc86 = 3 };
  if (lua_toboolean(L,3)) {
    
    /* return codepoint_to_utf8((n1 - 0xd800) * 0x400 + (n2 - 0xdc00) + 0x10000) */
    const int lc87 = lua_gettop(L);
    lc_getupvalue(L,lua_upvalueindex(1),1,14);
    lua_pushnumber(L,55296);
    lc_sub(L,2,-1);
    lua_remove(L,-2);
    lua_pushnumber(L,1024);
    lc_mul(L,-2,-1);
    lua_remove(L,-2);
    lua_remove(L,-2);
    lua_pushnumber(L,56320);
    lc_sub(L,3,-1);
    lua_remove(L,-2);
    lc_add(L,-2,-1);
    lua_remove(L,-2);
    lua_remove(L,-2);
    lua_pushnumber(L,65536);
    lc_add(L,-2,-1);
    lua_remove(L,-2);
    lua_remove(L,-2);
    lua_call(L,1,LUA_MULTRET);
    return (lua_gettop(L) - lc87);
  }
  else {
    
    /* else
     * return codepoint_to_utf8(n1) */
    const int lc88 = lua_gettop(L);
    lc_getupvalue(L,lua_upvalueindex(1),1,14);
    lua_pushvalue(L,2);
    lua_call(L,1,LUA_MULTRET);
    return (lua_gettop(L) - lc88);
  }
  lua_settop(L,lc86);
  return 0;
}


/* name: parse_string
 * function(str, i) */
static int lcf1_parse_string (lua_State * L) {
  enum { lc_nformalargs = 2 };
  lua_settop(L,2);
  
  /* local res = "" */
  lua_pushliteral(L,"");
  
  /* local j = i + 1 */
  lua_pushnumber(L,1);
  lc_add(L,2,-1);
  lua_remove(L,-2);
  
  /* local k = j */
  lua_pushvalue(L,4);
  
  /* while j <= #str do */
  enum { lc89 = 5 };
  while (1) {
    const double lc90 = lua_objlen(L,1);
    lua_pushnumber(L,lc90);
    const int lc91 = lc_le(L,4,-1);
    lua_pop(L,1);
    lua_pushboolean(L,lc91);
    if (!(lua_toboolean(L,-1))) {
      break;
    }
    lua_pop(L,1);
    
    /* local x = str:byte(j) */
    lua_pushvalue(L,1);
    lua_pushliteral(L,"byte");
    lua_gettable(L,-2);
    lua_insert(L,-2);
    lua_pushvalue(L,4);
    lua_call(L,2,1);
    
    /* if x < 32 then */
    enum { lc92 = 6 };
    lua_pushnumber(L,32);
    const int lc93 = lua_lessthan(L,6,-1);
    lua_pop(L,1);
    lua_pushboolean(L,lc93);
    const int lc94 = lua_toboolean(L,-1);
    lua_pop(L,1);
    if (lc94) {
      
      /* decode_error(str, j, "control character in string") */
      lc_getupvalue(L,lua_upvalueindex(1),2,13);
      lua_pushvalue(L,1);
      lua_pushvalue(L,4);
      lua_pushliteral(L,"control character in string");
      lua_call(L,3,0);
    }
    else {
      
      /* elseif x == 92 then */
      enum { lc95 = 6 };
      lua_pushnumber(L,92);
      const int lc96 = lua_equal(L,6,-1);
      lua_pop(L,1);
      lua_pushboolean(L,lc96);
      const int lc97 = lua_toboolean(L,-1);
      lua_pop(L,1);
      if (lc97) {
        
        /* -- `\`: Escape
         * res = res .. str:sub(k, j - 1) */
        lua_pushvalue(L,3);
        lua_pushvalue(L,1);
        lua_pushliteral(L,"sub");
        lua_gettable(L,-2);
        lua_insert(L,-2);
        lua_pushvalue(L,5);
        lua_pushnumber(L,1);
        lc_sub(L,4,-1);
        lua_remove(L,-2);
        lua_call(L,3,1);
        lua_concat(L,2);
        lua_replace(L,3);
        
        /* j = j + 1 */
        lua_pushnumber(L,1);
        lc_add(L,4,-1);
        lua_remove(L,-2);
        lua_replace(L,4);
        
        /* local c = str:sub(j, j) */
        lua_pushvalue(L,1);
        lua_pushliteral(L,"sub");
        lua_gettable(L,-2);
        lua_insert(L,-2);
        lua_pushvalue(L,4);
        lua_pushvalue(L,4);
        lua_call(L,3,1);
        
        /* if c == "u" then */
        enum { lc98 = 7 };
        lua_pushliteral(L,"u");
        const int lc99 = lua_equal(L,7,-1);
        lua_pop(L,1);
        lua_pushboolean(L,lc99);
        const int lc100 = lua_toboolean(L,-1);
        lua_pop(L,1);
        if (lc100) {
          
          /* local hex = str:match("^[dD][89aAbB]%x%x\\u%x%x%x%x", j + 1)
           *                  or str:match("^%x%x%x%x", j + 1)
           *                  or decode_error(str, j - 1, "invalid unicode escape in string") */
          lua_pushvalue(L,1);
          lua_pushliteral(L,"match");
          lua_gettable(L,-2);
          lua_insert(L,-2);
          lua_pushliteral(L,"^[dD][89aAbB]%x%x\\u%x%x%x%x");
          lua_pushnumber(L,1);
          lc_add(L,4,-1);
          lua_remove(L,-2);
          lua_call(L,3,1);
          if (!(lua_toboolean(L,-1))) {
            lua_pop(L,1);
            lua_pushvalue(L,1);
            lua_pushliteral(L,"match");
            lua_gettable(L,-2);
            lua_insert(L,-2);
            lua_pushliteral(L,"^%x%x%x%x");
            lua_pushnumber(L,1);
            lc_add(L,4,-1);
            lua_remove(L,-2);
            lua_call(L,3,1);
          }
          if (!(lua_toboolean(L,-1))) {
            lua_pop(L,1);
            lc_getupvalue(L,lua_upvalueindex(1),2,13);
            lua_pushvalue(L,1);
            lua_pushnumber(L,1);
            lc_sub(L,4,-1);
            lua_remove(L,-2);
            lua_pushliteral(L,"invalid unicode escape in string");
            lua_call(L,3,1);
          }
          
          /* res = res .. parse_unicode_escape(hex) */
          lua_pushvalue(L,3);
          lc_getupvalue(L,lua_upvalueindex(1),0,15);
          lua_pushvalue(L,8);
          lua_call(L,1,1);
          lua_concat(L,2);
          lua_replace(L,3);
          
          /* j = j + #hex */
          const double lc101 = lua_objlen(L,8);
          lua_pushnumber(L,lc101);
          lc_add(L,4,-1);
          lua_remove(L,-2);
          lua_replace(L,4);
        }
        else {
          
          /* else
           * if not escape_chars[c] then */
          enum { lc102 = 7 };
          lc_getupvalue(L,lua_upvalueindex(1),6,9);
          lua_pushvalue(L,7);
          lua_gettable(L,-2);
          lua_remove(L,-2);
          lua_pushboolean(L,!(lua_toboolean(L,-1)));
          lua_remove(L,-2);
          const int lc103 = lua_toboolean(L,-1);
          lua_pop(L,1);
          if (lc103) {
            
            /* decode_error(str, j - 1, "invalid escape char '" .. c .. "' in string") */
            lc_getupvalue(L,lua_upvalueindex(1),2,13);
            lua_pushvalue(L,1);
            lua_pushnumber(L,1);
            lc_sub(L,4,-1);
            lua_remove(L,-2);
            lua_pushliteral(L,"invalid escape char '");
            lua_pushvalue(L,7);
            lua_pushliteral(L,"' in string");
            lua_concat(L,2);
            lua_concat(L,2);
            lua_call(L,3,0);
          }
          lua_settop(L,lc102);
          
          /* res = res .. escape_char_map_inv[c] */
          lua_pushvalue(L,3);
          lc_getupvalue(L,lua_upvalueindex(1),12,3);
          lua_pushvalue(L,7);
          lua_gettable(L,-2);
          lua_remove(L,-2);
          lua_concat(L,2);
          lua_replace(L,3);
        }
        lua_settop(L,lc98);
        
        /* k = j + 1 */
        lua_pushnumber(L,1);
        lc_add(L,4,-1);
        lua_remove(L,-2);
        lua_replace(L,5);
      }
      else {
        
        /* elseif x == 34 then */
        enum { lc104 = 6 };
        lua_pushnumber(L,34);
        const int lc105 = lua_equal(L,6,-1);
        lua_pop(L,1);
        lua_pushboolean(L,lc105);
        const int lc106 = lua_toboolean(L,-1);
        lua_pop(L,1);
        if (lc106) {
          
          /* -- `"`: End of string
           * res = res .. str:sub(k, j - 1) */
          lua_pushvalue(L,3);
          lua_pushvalue(L,1);
          lua_pushliteral(L,"sub");
          lua_gettable(L,-2);
          lua_insert(L,-2);
          lua_pushvalue(L,5);
          lua_pushnumber(L,1);
          lc_sub(L,4,-1);
          lua_remove(L,-2);
          lua_call(L,3,1);
          lua_concat(L,2);
          lua_replace(L,3);
          
          /* return res, j + 1 */
          lua_pushvalue(L,3);
          lua_pushnumber(L,1);
          lc_add(L,4,-1);
          lua_remove(L,-2);
          return 2;
        }
        lua_settop(L,lc104);
      }
      lua_settop(L,lc95);
    }
    lua_settop(L,lc92);
    
    /* j = j + 1 */
    lua_pushnumber(L,1);
    lc_add(L,4,-1);
    lua_remove(L,-2);
    lua_replace(L,4);
    
    /* internal: stack cleanup on scope exit */
    lua_pop(L,1);
  }
  lua_settop(L,lc89);
  
  /* decode_error(str, i, "expected closing quote for string") */
  lc_getupvalue(L,lua_upvalueindex(1),2,13);
  lua_pushvalue(L,1);
  lua_pushvalue(L,2);
  lua_pushliteral(L,"expected closing quote for string");
  lua_call(L,3,0);
  return 0;
}


/* name: parse_number
 * function(str, i) */
static int lcf1_parse_number (lua_State * L) {
  enum { lc_nformalargs = 2 };
  lua_settop(L,2);
  
  /* local x = next_char(str, i, delim_chars) */
  lc_getupvalue(L,lua_upvalueindex(1),3,12);
  lua_pushvalue(L,1);
  lua_pushvalue(L,2);
  lc_getupvalue(L,lua_upvalueindex(1),7,8);
  lua_call(L,3,1);
  
  /* local s = str:sub(i, x - 1) */
  lua_pushvalue(L,1);
  lua_pushliteral(L,"sub");
  lua_gettable(L,-2);
  lua_insert(L,-2);
  lua_pushvalue(L,2);
  lua_pushnumber(L,1);
  lc_sub(L,3,-1);
  lua_remove(L,-2);
  lua_call(L,3,1);
  
  /* local n = tonumber(s) */
  lua_getfield(L,LUA_ENVIRONINDEX,"tonumber");
  lua_pushvalue(L,4);
  lua_call(L,1,1);
  
  /* if not n then */
  enum { lc107 = 5 };
  lua_pushboolean(L,!(lua_toboolean(L,5)));
  const int lc108 = lua_toboolean(L,-1);
  lua_pop(L,1);
  if (lc108) {
    
    /* decode_error(str, i, "invalid number '" .. s .. "'") */
    lc_getupvalue(L,lua_upvalueindex(1),2,13);
    lua_pushvalue(L,1);
    lua_pushvalue(L,2);
    lua_pushliteral(L,"invalid number '");
    lua_pushvalue(L,4);
    lua_pushliteral(L,"'");
    lua_concat(L,2);
    lua_concat(L,2);
    lua_call(L,3,0);
  }
  lua_settop(L,lc107);
  
  /* return n, x */
  lua_pushvalue(L,5);
  lua_pushvalue(L,3);
  return 2;
}


/* name: parse_literal
 * function(str, i) */
static int lcf1_parse_literal (lua_State * L) {
  enum { lc_nformalargs = 2 };
  lua_settop(L,2);
  
  /* local x = next_char(str, i, delim_chars) */
  lc_getupvalue(L,lua_upvalueindex(1),3,12);
  lua_pushvalue(L,1);
  lua_pushvalue(L,2);
  lc_getupvalue(L,lua_upvalueindex(1),7,8);
  lua_call(L,3,1);
  
  /* local word = str:sub(i, x - 1) */
  lua_pushvalue(L,1);
  lua_pushliteral(L,"sub");
  lua_gettable(L,-2);
  lua_insert(L,-2);
  lua_pushvalue(L,2);
  lua_pushnumber(L,1);
  lc_sub(L,3,-1);
  lua_remove(L,-2);
  lua_call(L,3,1);
  
  /* if not literals[word] then */
  enum { lc109 = 4 };
  lc_getupvalue(L,lua_upvalueindex(1),5,10);
  lua_pushvalue(L,4);
  lua_gettable(L,-2);
  lua_remove(L,-2);
  lua_pushboolean(L,!(lua_toboolean(L,-1)));
  lua_remove(L,-2);
  const int lc110 = lua_toboolean(L,-1);
  lua_pop(L,1);
  if (lc110) {
    
    /* decode_error(str, i, "invalid literal '" .. word .. "'") */
    lc_getupvalue(L,lua_upvalueindex(1),2,13);
    lua_pushvalue(L,1);
    lua_pushvalue(L,2);
    lua_pushliteral(L,"invalid literal '");
    lua_pushvalue(L,4);
    lua_pushliteral(L,"'");
    lua_concat(L,2);
    lua_concat(L,2);
    lua_call(L,3,0);
  }
  lua_settop(L,lc109);
  
  /* return literal_map[word], x */
  lc_getupvalue(L,lua_upvalueindex(1),4,11);
  lua_pushvalue(L,4);
  lua_gettable(L,-2);
  lua_remove(L,-2);
  lua_pushvalue(L,3);
  return 2;
}


/* name: parse_array
 * function(str, i) */
static int lcf1_parse_array (lua_State * L) {
  enum { lc_nformalargs = 2 };
  lua_settop(L,2);
  
  /* local res = {} */
  lua_newtable(L);
  
  /* local n = 1 */
  lua_pushnumber(L,1);
  
  /* i = i + 1 */
  lua_pushnumber(L,1);
  lc_add(L,2,-1);
  lua_remove(L,-2);
  lua_replace(L,2);
  
  /* while 1 do */
  enum { lc111 = 4 };
  while (1) {
    lua_pushnumber(L,1);
    if (!(lua_toboolean(L,-1))) {
      break;
    }
    lua_pop(L,1);
    
    /* local x */
    lua_settop(L,(lua_gettop(L) + 1));
    
    /* i = next_char(str, i, space_chars, true) */
    lc_getupvalue(L,lua_upvalueindex(1),3,12);
    lua_pushvalue(L,1);
    lua_pushvalue(L,2);
    lc_getupvalue(L,lua_upvalueindex(1),8,7);
    lua_pushboolean(L,1);
    lua_call(L,4,1);
    lua_replace(L,2);
    
    /* -- Empty / end of array?
     * if str:sub(i, i) == "]" then */
    enum { lc112 = 5 };
    lua_pushvalue(L,1);
    lua_pushliteral(L,"sub");
    lua_gettable(L,-2);
    lua_insert(L,-2);
    lua_pushvalue(L,2);
    lua_pushvalue(L,2);
    lua_call(L,3,1);
    lua_pushliteral(L,"]");
    const int lc113 = lua_equal(L,-2,-1);
    lua_pop(L,2);
    lua_pushboolean(L,lc113);
    const int lc114 = lua_toboolean(L,-1);
    lua_pop(L,1);
    if (lc114) {
      
      /* i = i + 1 */
      lua_pushnumber(L,1);
      lc_add(L,2,-1);
      lua_remove(L,-2);
      lua_replace(L,2);
      
      /* break */
      break;
    }
    lua_settop(L,lc112);
    
    /* -- Read token
     * x, i = parse(str, i) */
    lc_getupvalue(L,lua_upvalueindex(1),9,6);
    lua_pushvalue(L,1);
    lua_pushvalue(L,2);
    lua_call(L,2,2);
    lua_replace(L,2);
    lua_replace(L,5);
    
    /* res[n] = x */
    lua_pushvalue(L,5);
    lua_pushvalue(L,4);
    lua_insert(L,-2);
    lua_settable(L,3);
    
    /* n = n + 1 */
    lua_pushnumber(L,1);
    lc_add(L,4,-1);
    lua_remove(L,-2);
    lua_replace(L,4);
    
    /* -- Next token
     * i = next_char(str, i, space_chars, true) */
    lc_getupvalue(L,lua_upvalueindex(1),3,12);
    lua_pushvalue(L,1);
    lua_pushvalue(L,2);
    lc_getupvalue(L,lua_upvalueindex(1),8,7);
    lua_pushboolean(L,1);
    lua_call(L,4,1);
    lua_replace(L,2);
    
    /* local chr = str:sub(i, i) */
    lua_pushvalue(L,1);
    lua_pushliteral(L,"sub");
    lua_gettable(L,-2);
    lua_insert(L,-2);
    lua_pushvalue(L,2);
    lua_pushvalue(L,2);
    lua_call(L,3,1);
    
    /* i = i + 1 */
    lua_pushnumber(L,1);
    lc_add(L,2,-1);
    lua_remove(L,-2);
    lua_replace(L,2);
    
    /* if chr == "]" then */
    enum { lc115 = 6 };
    lua_pushliteral(L,"]");
    const int lc116 = lua_equal(L,6,-1);
    lua_pop(L,1);
    lua_pushboolean(L,lc116);
    const int lc117 = lua_toboolean(L,-1);
    lua_pop(L,1);
    if (lc117) {
      
      /* break */
      break;
    }
    lua_settop(L,lc115);
    
    /* if chr ~= "," then */
    enum { lc118 = 6 };
    lua_pushliteral(L,",");
    const int lc119 = lua_equal(L,6,-1);
    lua_pop(L,1);
    lua_pushboolean(L,lc119);
    lua_pushboolean(L,!(lua_toboolean(L,-1)));
    lua_remove(L,-2);
    const int lc120 = lua_toboolean(L,-1);
    lua_pop(L,1);
    if (lc120) {
      
      /* decode_error(str, i, "expected ']' or ','") */
      lc_getupvalue(L,lua_upvalueindex(1),2,13);
      lua_pushvalue(L,1);
      lua_pushvalue(L,2);
      lua_pushliteral(L,"expected ']' or ','");
      lua_call(L,3,0);
    }
    lua_settop(L,lc118);
    
    /* internal: stack cleanup on scope exit */
    lua_pop(L,2);
  }
  lua_settop(L,lc111);
  
  /* return res, i */
  lua_pushvalue(L,3);
  lua_pushvalue(L,2);
  return 2;
}


/* name: parse_object
 * function(str, i) */
static int lcf1_parse_object (lua_State * L) {
  enum { lc_nformalargs = 2 };
  lua_settop(L,2);
  
  /* local res = {} */
  lua_newtable(L);
  
  /* i = i + 1 */
  lua_pushnumber(L,1);
  lc_add(L,2,-1);
  lua_remove(L,-2);
  lua_replace(L,2);
  
  /* while 1 do */
  enum { lc121 = 3 };
  while (1) {
    lua_pushnumber(L,1);
    if (!(lua_toboolean(L,-1))) {
      break;
    }
    lua_pop(L,1);
    
    /* local key, val */
    lua_settop(L,(lua_gettop(L) + 2));
    
    /* i = next_char(str, i, space_chars, true) */
    lc_getupvalue(L,lua_upvalueindex(1),3,12);
    lua_pushvalue(L,1);
    lua_pushvalue(L,2);
    lc_getupvalue(L,lua_upvalueindex(1),8,7);
    lua_pushboolean(L,1);
    lua_call(L,4,1);
    lua_replace(L,2);
    
    /* -- Empty / end of object?
     * if str:sub(i, i) == "}" then */
    enum { lc122 = 5 };
    lua_pushvalue(L,1);
    lua_pushliteral(L,"sub");
    lua_gettable(L,-2);
    lua_insert(L,-2);
    lua_pushvalue(L,2);
    lua_pushvalue(L,2);
    lua_call(L,3,1);
    lua_pushliteral(L,"}");
    const int lc123 = lua_equal(L,-2,-1);
    lua_pop(L,2);
    lua_pushboolean(L,lc123);
    const int lc124 = lua_toboolean(L,-1);
    lua_pop(L,1);
    if (lc124) {
      
      /* i = i + 1 */
      lua_pushnumber(L,1);
      lc_add(L,2,-1);
      lua_remove(L,-2);
      lua_replace(L,2);
      
      /* break */
      break;
    }
    lua_settop(L,lc122);
    
    /* -- Read key
     * if str:sub(i, i) ~= '"' then */
    enum { lc125 = 5 };
    lua_pushvalue(L,1);
    lua_pushliteral(L,"sub");
    lua_gettable(L,-2);
    lua_insert(L,-2);
    lua_pushvalue(L,2);
    lua_pushvalue(L,2);
    lua_call(L,3,1);
    lua_pushliteral(L,"\"");
    const int lc126 = lua_equal(L,-2,-1);
    lua_pop(L,2);
    lua_pushboolean(L,lc126);
    lua_pushboolean(L,!(lua_toboolean(L,-1)));
    lua_remove(L,-2);
    const int lc127 = lua_toboolean(L,-1);
    lua_pop(L,1);
    if (lc127) {
      
      /* decode_error(str, i, "expected string for key") */
      lc_getupvalue(L,lua_upvalueindex(1),2,13);
      lua_pushvalue(L,1);
      lua_pushvalue(L,2);
      lua_pushliteral(L,"expected string for key");
      lua_call(L,3,0);
    }
    lua_settop(L,lc125);
    
    /* key, i = parse(str, i) */
    lc_getupvalue(L,lua_upvalueindex(1),9,6);
    lua_pushvalue(L,1);
    lua_pushvalue(L,2);
    lua_call(L,2,2);
    lua_replace(L,2);
    lua_replace(L,4);
    
    /* -- Read ':' delimiter
     * i = next_char(str, i, space_chars, true) */
    lc_getupvalue(L,lua_upvalueindex(1),3,12);
    lua_pushvalue(L,1);
    lua_pushvalue(L,2);
    lc_getupvalue(L,lua_upvalueindex(1),8,7);
    lua_pushboolean(L,1);
    lua_call(L,4,1);
    lua_replace(L,2);
    
    /* if str:sub(i, i) ~= ":" then */
    enum { lc128 = 5 };
    lua_pushvalue(L,1);
    lua_pushliteral(L,"sub");
    lua_gettable(L,-2);
    lua_insert(L,-2);
    lua_pushvalue(L,2);
    lua_pushvalue(L,2);
    lua_call(L,3,1);
    lua_pushliteral(L,":");
    const int lc129 = lua_equal(L,-2,-1);
    lua_pop(L,2);
    lua_pushboolean(L,lc129);
    lua_pushboolean(L,!(lua_toboolean(L,-1)));
    lua_remove(L,-2);
    const int lc130 = lua_toboolean(L,-1);
    lua_pop(L,1);
    if (lc130) {
      
      /* decode_error(str, i, "expected ':' after key") */
      lc_getupvalue(L,lua_upvalueindex(1),2,13);
      lua_pushvalue(L,1);
      lua_pushvalue(L,2);
      lua_pushliteral(L,"expected ':' after key");
      lua_call(L,3,0);
    }
    lua_settop(L,lc128);
    
    /* i = next_char(str, i + 1, space_chars, true) */
    lc_getupvalue(L,lua_upvalueindex(1),3,12);
    lua_pushvalue(L,1);
    lua_pushnumber(L,1);
    lc_add(L,2,-1);
    lua_remove(L,-2);
    lc_getupvalue(L,lua_upvalueindex(1),8,7);
    lua_pushboolean(L,1);
    lua_call(L,4,1);
    lua_replace(L,2);
    
    /* -- Read value
     * val, i = parse(str, i) */
    lc_getupvalue(L,lua_upvalueindex(1),9,6);
    lua_pushvalue(L,1);
    lua_pushvalue(L,2);
    lua_call(L,2,2);
    lua_replace(L,2);
    lua_replace(L,5);
    
    /* -- Set
     * res[key] = val */
    lua_pushvalue(L,5);
    lua_pushvalue(L,4);
    lua_insert(L,-2);
    lua_settable(L,3);
    
    /* -- Next token
     * i = next_char(str, i, space_chars, true) */
    lc_getupvalue(L,lua_upvalueindex(1),3,12);
    lua_pushvalue(L,1);
    lua_pushvalue(L,2);
    lc_getupvalue(L,lua_upvalueindex(1),8,7);
    lua_pushboolean(L,1);
    lua_call(L,4,1);
    lua_replace(L,2);
    
    /* local chr = str:sub(i, i) */
    lua_pushvalue(L,1);
    lua_pushliteral(L,"sub");
    lua_gettable(L,-2);
    lua_insert(L,-2);
    lua_pushvalue(L,2);
    lua_pushvalue(L,2);
    lua_call(L,3,1);
    
    /* i = i + 1 */
    lua_pushnumber(L,1);
    lc_add(L,2,-1);
    lua_remove(L,-2);
    lua_replace(L,2);
    
    /* if chr == "}" then */
    enum { lc131 = 6 };
    lua_pushliteral(L,"}");
    const int lc132 = lua_equal(L,6,-1);
    lua_pop(L,1);
    lua_pushboolean(L,lc132);
    const int lc133 = lua_toboolean(L,-1);
    lua_pop(L,1);
    if (lc133) {
      
      /* break */
      break;
    }
    lua_settop(L,lc131);
    
    /* if chr ~= "," then */
    enum { lc134 = 6 };
    lua_pushliteral(L,",");
    const int lc135 = lua_equal(L,6,-1);
    lua_pop(L,1);
    lua_pushboolean(L,lc135);
    lua_pushboolean(L,!(lua_toboolean(L,-1)));
    lua_remove(L,-2);
    const int lc136 = lua_toboolean(L,-1);
    lua_pop(L,1);
    if (lc136) {
      
      /* decode_error(str, i, "expected '}' or ','") */
      lc_getupvalue(L,lua_upvalueindex(1),2,13);
      lua_pushvalue(L,1);
      lua_pushvalue(L,2);
      lua_pushliteral(L,"expected '}' or ','");
      lua_call(L,3,0);
    }
    lua_settop(L,lc134);
    
    /* internal: stack cleanup on scope exit */
    lua_pop(L,3);
  }
  lua_settop(L,lc121);
  
  /* return res, i */
  lua_pushvalue(L,3);
  lua_pushvalue(L,2);
  return 2;
}


/* name: parse
 * function(str, idx) */
static int lcf1_parse (lua_State * L) {
  enum { lc_nformalargs = 2 };
  lua_settop(L,2);
  
  /* local chr = str:sub(idx, idx) */
  lua_pushvalue(L,1);
  lua_pushliteral(L,"sub");
  lua_gettable(L,-2);
  lua_insert(L,-2);
  lua_pushvalue(L,2);
  lua_pushvalue(L,2);
  lua_call(L,3,1);
  
  /* local f = char_func_map[chr] */
  lc_getupvalue(L,lua_upvalueindex(1),0,16);
  lua_pushvalue(L,3);
  lua_gettable(L,-2);
  lua_remove(L,-2);
  
  /* if f then */
  enum { lc138 = 4 };
  if (lua_toboolean(L,4)) {
    
    /* return f(str, idx) */
    const int lc139 = lua_gettop(L);
    lua_pushvalue(L,4);
    lua_pushvalue(L,1);
    lua_pushvalue(L,2);
    lua_call(L,2,LUA_MULTRET);
    return (lua_gettop(L) - lc139);
  }
  lua_settop(L,lc138);
  
  /* decode_error(str, idx, "unexpected character '" .. chr .. "'") */
  lc_getupvalue(L,lua_upvalueindex(1),3,13);
  lua_pushvalue(L,1);
  lua_pushvalue(L,2);
  lua_pushliteral(L,"unexpected character '");
  lua_pushvalue(L,3);
  lua_pushliteral(L,"'");
  lua_concat(L,2);
  lua_concat(L,2);
  lua_call(L,3,0);
  return 0;
}


/* name: json.decode
 * function(str) */
static int lcf1_json_decode (lua_State * L) {
  enum { lc_nformalargs = 1 };
  lua_settop(L,1);
  
  /* if type(str) ~= "string" then */
  enum { lc140 = 1 };
  lua_getfield(L,LUA_ENVIRONINDEX,"type");
  lua_pushvalue(L,1);
  lua_call(L,1,1);
  lua_pushliteral(L,"string");
  const int lc141 = lua_equal(L,-2,-1);
  lua_pop(L,2);
  lua_pushboolean(L,lc141);
  lua_pushboolean(L,!(lua_toboolean(L,-1)));
  lua_remove(L,-2);
  const int lc142 = lua_toboolean(L,-1);
  lua_pop(L,1);
  if (lc142) {
    
    /* error("expected argument of type string, got " .. type(str)) */
    lua_getfield(L,LUA_ENVIRONINDEX,"error");
    lua_pushliteral(L,"expected argument of type string, got ");
    lua_getfield(L,LUA_ENVIRONINDEX,"type");
    lua_pushvalue(L,1);
    lua_call(L,1,1);
    lua_concat(L,2);
    lua_call(L,1,0);
  }
  lua_settop(L,lc140);
  
  /* local res, idx = parse(str, next_char(str, 1, space_chars, true)) */
  lc_getupvalue(L,lua_upvalueindex(1),10,6);
  const int lc143 = lua_gettop(L);
  lua_pushvalue(L,1);
  lc_getupvalue(L,lua_upvalueindex(1),4,12);
  lua_pushvalue(L,1);
  lua_pushnumber(L,1);
  lc_getupvalue(L,lua_upvalueindex(1),9,7);
  lua_pushboolean(L,1);
  lua_call(L,4,LUA_MULTRET);
  lua_call(L,(lua_gettop(L) - lc143),2);
  
  /* idx = next_char(str, idx, space_chars, true) */
  lc_getupvalue(L,lua_upvalueindex(1),4,12);
  lua_pushvalue(L,1);
  lua_pushvalue(L,3);
  lc_getupvalue(L,lua_upvalueindex(1),9,7);
  lua_pushboolean(L,1);
  lua_call(L,4,1);
  lua_replace(L,3);
  
  /* if idx <= #str then */
  enum { lc144 = 3 };
  const double lc145 = lua_objlen(L,1);
  lua_pushnumber(L,lc145);
  const int lc146 = lc_le(L,3,-1);
  lua_pop(L,1);
  lua_pushboolean(L,lc146);
  const int lc147 = lua_toboolean(L,-1);
  lua_pop(L,1);
  if (lc147) {
    
    /* decode_error(str, idx, "trailing garbage") */
    lc_getupvalue(L,lua_upvalueindex(1),3,13);
    lua_pushvalue(L,1);
    lua_pushvalue(L,3);
    lua_pushliteral(L,"trailing garbage");
    lua_call(L,3,0);
  }
  lua_settop(L,lc144);
  
  /* return res */
  lua_pushvalue(L,2);
  return 1;
}

static int lcf_register (lua_State * L) {
  //lua_createtable(L,0,1);
  /* check whether lib already exists */
  luaL_findtable(L, LUA_REGISTRYINDEX, "_LOADED", 1);
  lua_getfield(L, -1, LUA_JSONLIBNAME); /* get _LOADED["json"] */
  if (!lua_istable(L, -1))
  {                  /* not found? */
    lua_pop(L, 1); /* remove previous result */
    /* try global variable (and create one if it does not exist) */
    if (luaL_findtable(L, LUA_GLOBALSINDEX, LUA_JSONLIBNAME, 1) != NULL)
        luaL_error(L, "name conflict for module '%s'", LUA_JSONLIBNAME);
    lua_pushvalue(L, -1);
    lua_setfield(L, -3, LUA_JSONLIBNAME); /* _LOADED["json"] = new table */
  }
  lua_remove(L, -2); /* remove _LOADED table */
  return 1;
}

/* name: (main)
 * function(...) */
static int lcf_main (lua_State * L) {
  lua_checkstack(L,37);
  enum { lc_nformalargs = 0 };
  const int lc_nactualargs = lua_gettop(L);
  const int lc_nextra = (lc_nactualargs - lc_nformalargs);
  
  /* --
   * -- json.lua
   * --
   * -- Copyright (c) 2020 rxi
   * --
   * -- Permission is hereby granted, free of charge, to any person obtaining a copy of
   * -- this software and associated documentation files (the "Software"), to deal in
   * -- the Software without restriction, including without limitation the rights to
   * -- use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
   * -- of the Software, and to permit persons to whom the Software is furnished to do
   * -- so, subject to the following conditions:
   * --
   * -- The above copyright notice and this permission notice shall be included in all
   * -- copies or substantial portions of the Software.
   * --
   * -- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   * -- IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   * -- FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   * -- AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   * -- LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   * -- OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   * -- SOFTWARE.
   * --
   * local json = { _version = "0.1.2" } */
  lcf_register(L);
  
  /* -------------------------------------------------------------------------------
   * -- Encode
   * -------------------------------------------------------------------------------
   * local encode */
  lc_newclosuretable(L,lua_upvalueindex(1));
  enum { lc1 = 2 };
  lua_settop(L,(lua_gettop(L) + 1));
  lua_rawseti(L,(lc1 + lc_nextra),1);
  
  /* local escape_char_map = {
   *   [ "\\" ] = "\\",
   *   [ "\"" ] = "\"",
   *   [ "\b" ] = "b",
   *   [ "\f" ] = "f",
   *   [ "\n" ] = "n",
   *   [ "\r" ] = "r",
   *   [ "\t" ] = "t",
   * } */
  lc_newclosuretable(L,(lc1 + lc_nextra));
  enum { lc2 = 3 };
  lua_createtable(L,0,7);
  lua_pushliteral(L,"\\");
  lua_pushliteral(L,"\\");
  lua_rawset(L,-3);
  lua_pushliteral(L,"\"");
  lua_pushliteral(L,"\"");
  lua_rawset(L,-3);
  lua_pushliteral(L,"");
  lua_pushliteral(L,"b");
  lua_rawset(L,-3);
  lua_pushliteral(L,"");
  lua_pushliteral(L,"f");
  lua_rawset(L,-3);
  lua_pushliteral(L,"\n");
  lua_pushliteral(L,"n");
  lua_rawset(L,-3);
  lua_pushliteral(L,"\r");
  lua_pushliteral(L,"r");
  lua_rawset(L,-3);
  lua_pushliteral(L,"	");
  lua_pushliteral(L,"t");
  lua_rawset(L,-3);
  lua_rawseti(L,(lc2 + lc_nextra),2);
  
  /* local escape_char_map_inv = { [ "/" ] = "/" } */
  lc_newclosuretable(L,(lc2 + lc_nextra));
  enum { lc3 = 4 };
  lua_createtable(L,0,1);
  lua_pushliteral(L,"/");
  lua_pushliteral(L,"/");
  lua_rawset(L,-3);
  lua_rawseti(L,(lc3 + lc_nextra),3);
  
  /* for k, v in pairs(escape_char_map) do
   * internal: local f, s, var = explist */
  enum { lc4 = 4 };
  lua_getfield(L,LUA_ENVIRONINDEX,"pairs");
  lc_getupvalue(L,(lc3 + lc_nextra),1,2);
  lua_call(L,1,3);
  while (1) {
    
    /* internal: local var_1, ..., var_n = f(s, var)
     *           if var_1 == nil then break end
     *           var = var_1 */
    lua_pushvalue(L,-3);
    lua_pushvalue(L,-3);
    lua_pushvalue(L,-3);
    lua_call(L,2,2);
    if (lua_isnil(L,-2)) {
      break;
    }
    lua_pushvalue(L,-2);
    lua_replace(L,-4);
    
    /* internal: local k with idx 8
     * internal: local v with idx 9 */
    
    
    /* escape_char_map_inv[v] = k */
    lua_pushvalue(L,(8 + lc_nextra));
    lc_getupvalue(L,(lc3 + lc_nextra),0,3);
    lua_insert(L,-2);
    lua_pushvalue(L,(9 + lc_nextra));
    lua_insert(L,-2);
    lua_settable(L,-3);
    lua_pop(L,1);
    
    /* internal: stack cleanup on scope exit */
    lua_pop(L,2);
  }
  lua_settop(L,(lc4 + lc_nextra));
  
  /* local function escape_char(c)
   *   return "\\" .. (escape_char_map[c] or string.format("u%04x", c:byte()))
   * end */
  lc_newclosuretable(L,(lc3 + lc_nextra));
  enum { lc5 = 5 };
  lua_pushvalue(L,(lc5 + lc_nextra));
  lua_pushcclosure(L,lcf1_escape_char, "escape_char", 1);
  lua_rawseti(L,(lc5 + lc_nextra),4);
  
  /* local function encode_nil(val)
   *   return "null"
   * end */
  lua_pushcfunction(L, lcf1_encode_nil, "encode_nil");
  
  /* local function encode_table(val, stack)
   *   local res = {}
   *   stack = stack or {}
   * 
   *   -- Circular reference?
   *   if stack[val] then error("circular reference") end
   * 
   *   stack[val] = true
   * 
   *   if rawget(val, 1) ~= nil or next(val) == nil then
   *     -- Treat as array -- check keys are valid and it is not sparse
   *     local n = 0
   *     for k in pairs(val) do
   *       if type(k) ~= "number" then
   *         error("invalid table: mixed or invalid key types")
   *       end
   *       n = n + 1
   *     end
   *     if n ~= #val then
   *       error("invalid table: sparse array")
   *     end
   *     -- Encode
   *     for i, v in ipairs(val) do
   *       table.insert(res, encode(v, stack))
   *     end
   *     stack[val] = nil
   *     return "[" .. table.concat(res, ",") .. "]"
   * 
   *   else
   *     -- Treat as an object
   *     for k, v in pairs(val) do
   *       if type(k) ~= "string" then
   *         error("invalid table: mixed or invalid key types")
   *       end
   *       table.insert(res, encode(k, stack) .. ":" .. encode(v, stack))
   *     end
   *     stack[val] = nil
   *     return "{" .. table.concat(res, ",") .. "}"
   *   end
   * end */
  lua_pushvalue(L,(lc5 + lc_nextra));
  lua_pushcclosure(L, lcf1_encode_table, "encode_table", 1);
  
  /* local function encode_string(val)
   *   return '"' .. val:gsub('[%z\1-\31\\"]', escape_char) .. '"'
   * end */
  lua_pushvalue(L,(lc5 + lc_nextra));
  lua_pushcclosure(L, lcf1_encode_string, "encode_string", 1);
  
  /* local function encode_number(val)
   *   -- Check for NaN, -inf and inf
   *   if val ~= val or val <= -math.huge or val >= math.huge then
   *     error("unexpected number value '" .. tostring(val) .. "'")
   *   end
   *   return string.format("%.14g", val)
   * end */
  lua_pushcfunction(L, lcf1_encode_number, "encode_number");
  
  /* local type_func_map = {
   *   [ "nil"     ] = encode_nil,
   *   [ "table"   ] = encode_table,
   *   [ "string"  ] = encode_string,
   *   [ "number"  ] = encode_number,
   *   [ "boolean" ] = tostring,
   * } */
  lc_newclosuretable(L,(lc5 + lc_nextra));
  enum { lc33 = 10 };
  lua_createtable(L,0,5);
  lua_pushliteral(L,"nil");
  lua_pushvalue(L,(6 + lc_nextra));
  lua_rawset(L,-3);
  lua_pushliteral(L,"table");
  lua_pushvalue(L,(7 + lc_nextra));
  lua_rawset(L,-3);
  lua_pushliteral(L,"string");
  lua_pushvalue(L,(8 + lc_nextra));
  lua_rawset(L,-3);
  lua_pushliteral(L,"number");
  lua_pushvalue(L,(9 + lc_nextra));
  lua_rawset(L,-3);
  lua_pushliteral(L,"boolean");
  lua_getfield(L,LUA_ENVIRONINDEX,"tostring");
  lua_rawset(L,-3);
  lua_rawseti(L,(lc33 + lc_nextra),5);
  
  /* encode = function(val, stack)
   *   local t = type(val)
   *   local f = type_func_map[t]
   *   if f then
   *     return f(val, stack)
   *   end
   *   error("unexpected type '" .. t .. "'")
   * end */
  lua_pushvalue(L,(lc33 + lc_nextra));
  lua_pushcclosure(L, lcf1_encode, "encode", 1);
  lc_setupvalue(L,(lc33 + lc_nextra),4,1);
  
  /* function json.encode(val)
   *   return ( encode(val) )
   * end */
  lua_pushvalue(L,(lc33 + lc_nextra));
  lua_pushcclosure(L, lcf1_json_encode, "encode", 1);
  lua_pushliteral(L,"encode");
  lua_insert(L,-2);
  lua_settable(L,(1 + lc_nextra));
  
  /* -------------------------------------------------------------------------------
   * -- Decode
   * -------------------------------------------------------------------------------
   * local parse */
  lc_newclosuretable(L,(lc33 + lc_nextra));
  enum { lc36 = 11 };
  lua_settop(L,(lua_gettop(L) + 1));
  lua_rawseti(L,(lc36 + lc_nextra),6);
  
  /* local function create_set(...)
   *   local res = {}
   *   for i = 1, select("#", ...) do
   *     res[ select(i, ...) ] = true
   *   end
   *   return res
   * end */
  lua_pushcfunction(L, lcf1_create_set, "create_set");
  
  /* local space_chars   = create_set(" ", "\t", "\r", "\n") */
  lc_newclosuretable(L,(lc36 + lc_nextra));
  enum { lc43 = 13 };
  lua_pushvalue(L,(12 + lc_nextra));
  lua_pushliteral(L," ");
  lua_pushliteral(L,"	");
  lua_pushliteral(L,"\r");
  lua_pushliteral(L,"\n");
  lua_call(L,4,1);
  lua_rawseti(L,(lc43 + lc_nextra),7);
  
  /* local delim_chars   = create_set(" ", "\t", "\r", "\n", "]", "}", ",") */
  lc_newclosuretable(L,(lc43 + lc_nextra));
  enum { lc44 = 14 };
  lua_pushvalue(L,(12 + lc_nextra));
  lua_pushliteral(L," ");
  lua_pushliteral(L,"	");
  lua_pushliteral(L,"\r");
  lua_pushliteral(L,"\n");
  lua_pushliteral(L,"]");
  lua_pushliteral(L,"}");
  lua_pushliteral(L,",");
  lua_call(L,7,1);
  lua_rawseti(L,(lc44 + lc_nextra),8);
  
  /* local escape_chars  = create_set("\\", "/", '"', "b", "f", "n", "r", "t", "u") */
  lc_newclosuretable(L,(lc44 + lc_nextra));
  enum { lc45 = 15 };
  lua_pushvalue(L,(12 + lc_nextra));
  lua_pushliteral(L,"\\");
  lua_pushliteral(L,"/");
  lua_pushliteral(L,"\"");
  lua_pushliteral(L,"b");
  lua_pushliteral(L,"f");
  lua_pushliteral(L,"n");
  lua_pushliteral(L,"r");
  lua_pushliteral(L,"t");
  lua_pushliteral(L,"u");
  lua_call(L,9,1);
  lua_rawseti(L,(lc45 + lc_nextra),9);
  
  /* local literals      = create_set("true", "false", "null") */
  lc_newclosuretable(L,(lc45 + lc_nextra));
  enum { lc46 = 16 };
  lua_pushvalue(L,(12 + lc_nextra));
  lua_pushliteral(L,"true");
  lua_pushliteral(L,"false");
  lua_pushliteral(L,"null");
  lua_call(L,3,1);
  lua_rawseti(L,(lc46 + lc_nextra),10);
  
  /* local literal_map = {
   *   [ "true"  ] = true,
   *   [ "false" ] = false,
   *   [ "null"  ] = nil,
   * } */
  lc_newclosuretable(L,(lc46 + lc_nextra));
  enum { lc47 = 17 };
  lua_createtable(L,0,3);
  lua_pushliteral(L,"true");
  lua_pushboolean(L,1);
  lua_rawset(L,-3);
  lua_pushliteral(L,"false");
  lua_pushboolean(L,0);
  lua_rawset(L,-3);
  lua_pushliteral(L,"null");
  lua_pushnil(L);
  lua_rawset(L,-3);
  lua_rawseti(L,(lc47 + lc_nextra),11);
  
  /* local function next_char(str, idx, set, negate)
   *   for i = idx, #str do
   *     if set[str:sub(i, i)] ~= negate then
   *       return i
   *     end
   *   end
   *   return #str + 1
   * end */
  lc_newclosuretable(L,(lc47 + lc_nextra));
  enum { lc48 = 18 };
  lua_pushcfunction(L, lcf1_next_char, "next_char");
  lua_rawseti(L,(lc48 + lc_nextra),12);
  
  /* local function decode_error(str, idx, msg)
   *   local line_count = 1
   *   local col_count = 1
   *   for i = 1, idx - 1 do
   *     col_count = col_count + 1
   *     if str:sub(i, i) == "\n" then
   *       line_count = line_count + 1
   *       col_count = 1
   *     end
   *   end
   *   error( string.format("%s at line %d col %d", msg, line_count, col_count) )
   * end */
  lc_newclosuretable(L,(lc48 + lc_nextra));
  enum { lc58 = 19 };
  lua_pushcfunction(L, lcf1_decode_error, "decode_error");
  lua_rawseti(L,(lc58 + lc_nextra),13);
  
  /* local function codepoint_to_utf8(n)
   *   -- http://scripts.sil.org/cms/scripts/page.php?site_id=nrsi&id=iws-appendixa
   *   local f = math.floor
   *   if n <= 0x7f then
   *     return string.char(n)
   *   elseif n <= 0x7ff then
   *     return string.char(f(n / 64) + 192, n % 64 + 128)
   *   elseif n <= 0xffff then
   *     return string.char(f(n / 4096) + 224, f(n % 4096 / 64) + 128, n % 64 + 128)
   *   elseif n <= 0x10ffff then
   *     return string.char(f(n / 262144) + 240, f(n % 262144 / 4096) + 128,
   *                        f(n % 4096 / 64) + 128, n % 64 + 128)
   *   end
   *   error( string.format("invalid unicode codepoint '%x'", n) )
   * end */
  lc_newclosuretable(L,(lc58 + lc_nextra));
  enum { lc67 = 20 };
  lua_pushcfunction(L,lcf1_codepoint_to_utf8, "codepoint_to_utf8");
  lua_rawseti(L,(lc67 + lc_nextra),14);
  
  /* local function parse_unicode_escape(s)
   *   local n1 = tonumber( s:sub(1, 4),  16 )
   *   local n2 = tonumber( s:sub(7, 10), 16 )
   *    -- Surrogate pair?
   *   if n2 then
   *     return codepoint_to_utf8((n1 - 0xd800) * 0x400 + (n2 - 0xdc00) + 0x10000)
   *   else
   *     return codepoint_to_utf8(n1)
   *   end
   * end */
  lc_newclosuretable(L,(lc67 + lc_nextra));
  enum { lc85 = 21 };
  lua_pushvalue(L,(lc85 + lc_nextra));
  lua_pushcclosure(L,lcf1_parse_unicode_escape,"parse_unicode_escape",1);
  lua_rawseti(L,(lc85 + lc_nextra),15);
  
  /* local function parse_string(str, i)
   *   local res = ""
   *   local j = i + 1
   *   local k = j
   * 
   *   while j <= #str do
   *     local x = str:byte(j)
   * 
   *     if x < 32 then
   *       decode_error(str, j, "control character in string")
   * 
   *     elseif x == 92 then -- `\`: Escape
   *       res = res .. str:sub(k, j - 1)
   *       j = j + 1
   *       local c = str:sub(j, j)
   *       if c == "u" then
   *         local hex = str:match("^[dD][89aAbB]%x%x\\u%x%x%x%x", j + 1)
   *                  or str:match("^%x%x%x%x", j + 1)
   *                  or decode_error(str, j - 1, "invalid unicode escape in string")
   *         res = res .. parse_unicode_escape(hex)
   *         j = j + #hex
   *       else
   *         if not escape_chars[c] then
   *           decode_error(str, j - 1, "invalid escape char '" .. c .. "' in string")
   *         end
   *         res = res .. escape_char_map_inv[c]
   *       end
   *       k = j + 1
   * 
   *     elseif x == 34 then -- `"`: End of string
   *       res = res .. str:sub(k, j - 1)
   *       return res, j + 1
   *     end
   * 
   *     j = j + 1
   *   end
   * 
   *   decode_error(str, i, "expected closing quote for string")
   * end */
  lua_pushvalue(L,(lc85 + lc_nextra));
  lua_pushcclosure(L,lcf1_parse_string, "parse_string",1);
  
  /* local function parse_number(str, i)
   *   local x = next_char(str, i, delim_chars)
   *   local s = str:sub(i, x - 1)
   *   local n = tonumber(s)
   *   if not n then
   *     decode_error(str, i, "invalid number '" .. s .. "'")
   *   end
   *   return n, x
   * end */
  lua_pushvalue(L,(lc85 + lc_nextra));
  lua_pushcclosure(L,lcf1_parse_number,"parse_number",1);
  
  /* local function parse_literal(str, i)
   *   local x = next_char(str, i, delim_chars)
   *   local word = str:sub(i, x - 1)
   *   if not literals[word] then
   *     decode_error(str, i, "invalid literal '" .. word .. "'")
   *   end
   *   return literal_map[word], x
   * end */
  lua_pushvalue(L,(lc85 + lc_nextra));
  lua_pushcclosure(L,lcf1_parse_literal,"parse_literal",1);
  
  /* local function parse_array(str, i)
   *   local res = {}
   *   local n = 1
   *   i = i + 1
   *   while 1 do
   *     local x
   *     i = next_char(str, i, space_chars, true)
   *     -- Empty / end of array?
   *     if str:sub(i, i) == "]" then
   *       i = i + 1
   *       break
   *     end
   *     -- Read token
   *     x, i = parse(str, i)
   *     res[n] = x
   *     n = n + 1
   *     -- Next token
   *     i = next_char(str, i, space_chars, true)
   *     local chr = str:sub(i, i)
   *     i = i + 1
   *     if chr == "]" then break end
   *     if chr ~= "," then decode_error(str, i, "expected ']' or ','") end
   *   end
   *   return res, i
   * end */
  lua_pushvalue(L,(lc85 + lc_nextra));
  lua_pushcclosure(L,lcf1_parse_array,"parse_array",1);
  
  /* local function parse_object(str, i)
   *   local res = {}
   *   i = i + 1
   *   while 1 do
   *     local key, val
   *     i = next_char(str, i, space_chars, true)
   *     -- Empty / end of object?
   *     if str:sub(i, i) == "}" then
   *       i = i + 1
   *       break
   *     end
   *     -- Read key
   *     if str:sub(i, i) ~= '"' then
   *       decode_error(str, i, "expected string for key")
   *     end
   *     key, i = parse(str, i)
   *     -- Read ':' delimiter
   *     i = next_char(str, i, space_chars, true)
   *     if str:sub(i, i) ~= ":" then
   *       decode_error(str, i, "expected ':' after key")
   *     end
   *     i = next_char(str, i + 1, space_chars, true)
   *     -- Read value
   *     val, i = parse(str, i)
   *     -- Set
   *     res[key] = val
   *     -- Next token
   *     i = next_char(str, i, space_chars, true)
   *     local chr = str:sub(i, i)
   *     i = i + 1
   *     if chr == "}" then break end
   *     if chr ~= "," then decode_error(str, i, "expected '}' or ','") end
   *   end
   *   return res, i
   * end */
  lua_pushvalue(L,(lc85 + lc_nextra));
  lua_pushcclosure(L,lcf1_parse_object,"parse_object",1);
  
  /* local char_func_map = {
   *   [ '"' ] = parse_string,
   *   [ "0" ] = parse_number,
   *   [ "1" ] = parse_number,
   *   [ "2" ] = parse_number,
   *   [ "3" ] = parse_number,
   *   [ "4" ] = parse_number,
   *   [ "5" ] = parse_number,
   *   [ "6" ] = parse_number,
   *   [ "7" ] = parse_number,
   *   [ "8" ] = parse_number,
   *   [ "9" ] = parse_number,
   *   [ "-" ] = parse_number,
   *   [ "t" ] = parse_literal,
   *   [ "f" ] = parse_literal,
   *   [ "n" ] = parse_literal,
   *   [ "[" ] = parse_array,
   *   [ "{" ] = parse_object,
   * } */
  lc_newclosuretable(L,(lc85 + lc_nextra));
  enum { lc137 = 27 };
  lua_createtable(L,0,17);
  lua_pushliteral(L,"\"");
  lua_pushvalue(L,(22 + lc_nextra));
  lua_rawset(L,-3);
  lua_pushliteral(L,"0");
  lua_pushvalue(L,(23 + lc_nextra));
  lua_rawset(L,-3);
  lua_pushliteral(L,"1");
  lua_pushvalue(L,(23 + lc_nextra));
  lua_rawset(L,-3);
  lua_pushliteral(L,"2");
  lua_pushvalue(L,(23 + lc_nextra));
  lua_rawset(L,-3);
  lua_pushliteral(L,"3");
  lua_pushvalue(L,(23 + lc_nextra));
  lua_rawset(L,-3);
  lua_pushliteral(L,"4");
  lua_pushvalue(L,(23 + lc_nextra));
  lua_rawset(L,-3);
  lua_pushliteral(L,"5");
  lua_pushvalue(L,(23 + lc_nextra));
  lua_rawset(L,-3);
  lua_pushliteral(L,"6");
  lua_pushvalue(L,(23 + lc_nextra));
  lua_rawset(L,-3);
  lua_pushliteral(L,"7");
  lua_pushvalue(L,(23 + lc_nextra));
  lua_rawset(L,-3);
  lua_pushliteral(L,"8");
  lua_pushvalue(L,(23 + lc_nextra));
  lua_rawset(L,-3);
  lua_pushliteral(L,"9");
  lua_pushvalue(L,(23 + lc_nextra));
  lua_rawset(L,-3);
  lua_pushliteral(L,"-");
  lua_pushvalue(L,(23 + lc_nextra));
  lua_rawset(L,-3);
  lua_pushliteral(L,"t");
  lua_pushvalue(L,(24 + lc_nextra));
  lua_rawset(L,-3);
  lua_pushliteral(L,"f");
  lua_pushvalue(L,(24 + lc_nextra));
  lua_rawset(L,-3);
  lua_pushliteral(L,"n");
  lua_pushvalue(L,(24 + lc_nextra));
  lua_rawset(L,-3);
  lua_pushliteral(L,"[");
  lua_pushvalue(L,(25 + lc_nextra));
  lua_rawset(L,-3);
  lua_pushliteral(L,"{");
  lua_pushvalue(L,(26 + lc_nextra));
  lua_rawset(L,-3);
  lua_rawseti(L,(lc137 + lc_nextra),16);
  
  /* parse = function(str, idx)
   *   local chr = str:sub(idx, idx)
   *   local f = char_func_map[chr]
   *   if f then
   *     return f(str, idx)
   *   end
   *   decode_error(str, idx, "unexpected character '" .. chr .. "'")
   * end */
  lua_pushvalue(L,(lc137 + lc_nextra));
  lua_pushcclosure(L,lcf1_parse,"parse",1);
  lc_setupvalue(L,(lc137 + lc_nextra),10,6);
  
  /* function json.decode(str)
   *   if type(str) ~= "string" then
   *     error("expected argument of type string, got " .. type(str))
   *   end
   *   local res, idx = parse(str, next_char(str, 1, space_chars, true))
   *   idx = next_char(str, idx, space_chars, true)
   *   if idx <= #str then
   *     decode_error(str, idx, "trailing garbage")
   *   end
   *   return res
   * end */
  lua_pushvalue(L,(lc137 + lc_nextra));
  lua_pushcclosure(L,lcf1_json_decode,"decode",1);
  lua_pushliteral(L,"decode");
  lua_insert(L,-2);
  lua_settable(L,(1 + lc_nextra));
  
  /* return json */
  lua_pushvalue(L,(1 + lc_nextra));
  return 1;
}

LUALIB_API int luaopen_json(lua_State* L)
{
  lcf_main(L);

  return 1;
}