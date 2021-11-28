#include "lua.h"
#include "lapi.h"
#include "lualib.h"
#include "lc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* name: base64Encode
 * function(str) */
static int lcf1_base64Encode (lua_State * L) {
  enum { lc_nformalargs = 1 };
  lua_settop(L,1);
  
  /* local parts = {} */
  lua_newtable(L);
  
  /* local j = 1 */
  lua_pushnumber(L,1);
  
  /* for i = 1, #str, 3 do */
  lua_pushnumber(L,1);
  const double lc12 = lua_objlen(L,1);
  lua_pushnumber(L,lc12);
  lua_pushnumber(L,3);
  if (!(((lua_isnumber(L,-3) && lua_isnumber(L,-2)) && lua_isnumber(L,-1)))) {
    luaL_error(L,"'for' limit must be a number");
  }
  double lc9_var = lua_tonumber(L,-3);
  const double lc10_limit = lua_tonumber(L,-2);
  const double lc11_step = lua_tonumber(L,-1);
  lua_pop(L,3);
  enum { lc13 = 3 };
  while ((((lc11_step > 0) && (lc9_var <= lc10_limit)) || ((lc11_step <= 0) && (lc9_var >= lc10_limit)))) {
    
    /* internal: local i at index 4 */
    lua_pushnumber(L,lc9_var);
    
    /* local a, b, c = byte(str, i, i + 2) */
    lc_getupvalue(L,lua_upvalueindex(1),2,6);
    lua_pushvalue(L,1);
    lua_pushvalue(L,4);
    lua_pushnumber(L,2);
    lc_add(L,4,-1);
    lua_remove(L,-2);
    lua_call(L,3,3);
    
    /* parts[j] = char(
     *       -- Higher 6 bits of a
     *       byte(codes, rshift(a, 2) + 1),
     *       -- Lower 2 bits of a + high 4 bits of b
     *       byte(codes, bor(
     *         lshift(band(a, 3), 4),
     *         b and rshift(b, 4) or 0
     *       ) + 1),
     *       -- Low 4 bits of b + High 2 bits of c
     *       b and byte(codes, bor(
     *         lshift(band(b, 15), 2),
     *         c and rshift(c, 6) or 0
     *       ) + 1) or 61, -- 61 is '='
     *       -- Lower 6 bits of c
     *       c and byte(codes, band(c, 63) + 1) or 61 -- 61 is '='
     *     ) */
    lc_getupvalue(L,lua_upvalueindex(1),3,5);
    lc_getupvalue(L,lua_upvalueindex(1),2,6);
    lc_getupvalue(L,lua_upvalueindex(1),0,8);
    lc_getupvalue(L,lua_upvalueindex(1),7,1);
    lua_pushvalue(L,5);
    lua_pushnumber(L,2);
    lua_call(L,2,1);
    lua_pushnumber(L,1);
    lc_add(L,-2,-1);
    lua_remove(L,-2);
    lua_remove(L,-2);
    lua_call(L,2,1);
    lc_getupvalue(L,lua_upvalueindex(1),2,6);
    lc_getupvalue(L,lua_upvalueindex(1),0,8);
    lc_getupvalue(L,lua_upvalueindex(1),5,3);
    lc_getupvalue(L,lua_upvalueindex(1),6,2);
    lc_getupvalue(L,lua_upvalueindex(1),4,4);
    lua_pushvalue(L,5);
    lua_pushnumber(L,3);
    lua_call(L,2,1);
    lua_pushnumber(L,4);
    lua_call(L,2,1);
    lua_pushvalue(L,6);
    if (lua_toboolean(L,-1)) {
      lua_pop(L,1);
      lc_getupvalue(L,lua_upvalueindex(1),7,1);
      lua_pushvalue(L,6);
      lua_pushnumber(L,4);
      lua_call(L,2,1);
    }
    if (!(lua_toboolean(L,-1))) {
      lua_pop(L,1);
      lua_pushnumber(L,0);
    }
    lua_call(L,2,1);
    lua_pushnumber(L,1);
    lc_add(L,-2,-1);
    lua_remove(L,-2);
    lua_remove(L,-2);
    lua_call(L,2,1);
    lua_pushvalue(L,6);
    if (lua_toboolean(L,-1)) {
      lua_pop(L,1);
      lc_getupvalue(L,lua_upvalueindex(1),2,6);
      lc_getupvalue(L,lua_upvalueindex(1),0,8);
      lc_getupvalue(L,lua_upvalueindex(1),5,3);
      lc_getupvalue(L,lua_upvalueindex(1),6,2);
      lc_getupvalue(L,lua_upvalueindex(1),4,4);
      lua_pushvalue(L,6);
      lua_pushnumber(L,15);
      lua_call(L,2,1);
      lua_pushnumber(L,2);
      lua_call(L,2,1);
      lua_pushvalue(L,7);
      if (lua_toboolean(L,-1)) {
        lua_pop(L,1);
        lc_getupvalue(L,lua_upvalueindex(1),7,1);
        lua_pushvalue(L,7);
        lua_pushnumber(L,6);
        lua_call(L,2,1);
      }
      if (!(lua_toboolean(L,-1))) {
        lua_pop(L,1);
        lua_pushnumber(L,0);
      }
      lua_call(L,2,1);
      lua_pushnumber(L,1);
      lc_add(L,-2,-1);
      lua_remove(L,-2);
      lua_remove(L,-2);
      lua_call(L,2,1);
    }
    if (!(lua_toboolean(L,-1))) {
      lua_pop(L,1);
      lua_pushnumber(L,61);
    }
    lua_pushvalue(L,7);
    if (lua_toboolean(L,-1)) {
      lua_pop(L,1);
      lc_getupvalue(L,lua_upvalueindex(1),2,6);
      lc_getupvalue(L,lua_upvalueindex(1),0,8);
      lc_getupvalue(L,lua_upvalueindex(1),4,4);
      lua_pushvalue(L,7);
      lua_pushnumber(L,63);
      lua_call(L,2,1);
      lua_pushnumber(L,1);
      lc_add(L,-2,-1);
      lua_remove(L,-2);
      lua_remove(L,-2);
      lua_call(L,2,1);
    }
    if (!(lua_toboolean(L,-1))) {
      lua_pop(L,1);
      lua_pushnumber(L,61);
    }
    lua_call(L,4,1);
    lua_pushvalue(L,3);
    lua_insert(L,-2);
    lua_settable(L,2);
    
    /* j = j + 1 */
    lua_pushnumber(L,1);
    lc_add(L,3,-1);
    lua_remove(L,-2);
    lua_replace(L,3);
    
    /* internal: stack cleanup on scope exit */
    lua_pop(L,4);
    lc9_var += lc11_step;
  }
  lua_settop(L,lc13);
  
  /* return concat(parts) */
  const int lc14 = lua_gettop(L);
  lc_getupvalue(L,lua_upvalueindex(1),1,7);
  lua_pushvalue(L,2);
  lua_call(L,1,LUA_MULTRET);
  return (lua_gettop(L) - lc14);
}


/* name: base64Decode
 * function(data) */
static int lcf1_base64Decode (lua_State * L) {
  enum { lc_nformalargs = 1 };
  lua_settop(L,1);
  
  /* local bytes = {} */
  lua_newtable(L);
  
  /* local j = 1 */
  lua_pushnumber(L,1);
  
  /* for i = 1, #data, 4 do */
  lua_pushnumber(L,1);
  const double lc24 = lua_objlen(L,1);
  lua_pushnumber(L,lc24);
  lua_pushnumber(L,4);
  if (!(((lua_isnumber(L,-3) && lua_isnumber(L,-2)) && lua_isnumber(L,-1)))) {
    luaL_error(L,"'for' limit must be a number");
  }
  double lc21_var = lua_tonumber(L,-3);
  const double lc22_limit = lua_tonumber(L,-2);
  const double lc23_step = lua_tonumber(L,-1);
  lua_pop(L,3);
  enum { lc25 = 3 };
  while ((((lc23_step > 0) && (lc21_var <= lc22_limit)) || ((lc23_step <= 0) && (lc21_var >= lc22_limit)))) {
    
    /* internal: local i at index 4 */
    lua_pushnumber(L,lc21_var);
    
    /* local a = map[byte(data, i)] */
    lc_getupvalue(L,lua_upvalueindex(1),0,9);
    lc_getupvalue(L,lua_upvalueindex(1),3,6);
    lua_pushvalue(L,1);
    lua_pushvalue(L,4);
    lua_call(L,2,1);
    lua_gettable(L,-2);
    lua_remove(L,-2);
    
    /* local b = map[byte(data, i + 1)] */
    lc_getupvalue(L,lua_upvalueindex(1),0,9);
    lc_getupvalue(L,lua_upvalueindex(1),3,6);
    lua_pushvalue(L,1);
    lua_pushnumber(L,1);
    lc_add(L,4,-1);
    lua_remove(L,-2);
    lua_call(L,2,1);
    lua_gettable(L,-2);
    lua_remove(L,-2);
    
    /* local c = map[byte(data, i + 2)] */
    lc_getupvalue(L,lua_upvalueindex(1),0,9);
    lc_getupvalue(L,lua_upvalueindex(1),3,6);
    lua_pushvalue(L,1);
    lua_pushnumber(L,2);
    lc_add(L,4,-1);
    lua_remove(L,-2);
    lua_call(L,2,1);
    lua_gettable(L,-2);
    lua_remove(L,-2);
    
    /* local d = map[byte(data, i + 3)] */
    lc_getupvalue(L,lua_upvalueindex(1),0,9);
    lc_getupvalue(L,lua_upvalueindex(1),3,6);
    lua_pushvalue(L,1);
    lua_pushnumber(L,3);
    lc_add(L,4,-1);
    lua_remove(L,-2);
    lua_call(L,2,1);
    lua_gettable(L,-2);
    lua_remove(L,-2);
    
    /* -- higher 6 bits are the first char
     * -- lower 2 bits are upper 2 bits of second char
     * bytes[j] = char(bor(lshift(a, 2), rshift(b, 4))) */
    lc_getupvalue(L,lua_upvalueindex(1),4,5);
    const int lc26 = lua_gettop(L);
    lc_getupvalue(L,lua_upvalueindex(1),6,3);
    const int lc27 = lua_gettop(L);
    lc_getupvalue(L,lua_upvalueindex(1),7,2);
    lua_pushvalue(L,5);
    lua_pushnumber(L,2);
    lua_call(L,2,1);
    lc_getupvalue(L,lua_upvalueindex(1),8,1);
    lua_pushvalue(L,6);
    lua_pushnumber(L,4);
    lua_call(L,2,LUA_MULTRET);
    lua_call(L,(lua_gettop(L) - lc27),LUA_MULTRET);
    lua_call(L,(lua_gettop(L) - lc26),1);
    lua_pushvalue(L,3);
    lua_insert(L,-2);
    lua_settable(L,2);
    
    /* -- if the third char is not padding, we have a second byte
     * if c < 64 then */
    enum { lc28 = 8 };
    lua_pushnumber(L,64);
    const int lc29 = lua_lessthan(L,7,-1);
    lua_pop(L,1);
    lua_pushboolean(L,lc29);
    const int lc30 = lua_toboolean(L,-1);
    lua_pop(L,1);
    if (lc30) {
      
      /* -- high 4 bits come from lower 4 bits in b
       * -- low 4 bits come from high 4 bits in c
       * bytes[j + 1] = char(bor(lshift(band(b, 0xf), 4), rshift(c, 2))) */
      lc_getupvalue(L,lua_upvalueindex(1),4,5);
      const int lc31 = lua_gettop(L);
      lc_getupvalue(L,lua_upvalueindex(1),6,3);
      const int lc32 = lua_gettop(L);
      lc_getupvalue(L,lua_upvalueindex(1),7,2);
      lc_getupvalue(L,lua_upvalueindex(1),5,4);
      lua_pushvalue(L,6);
      lua_pushnumber(L,15);
      lua_call(L,2,1);
      lua_pushnumber(L,4);
      lua_call(L,2,1);
      lc_getupvalue(L,lua_upvalueindex(1),8,1);
      lua_pushvalue(L,7);
      lua_pushnumber(L,2);
      lua_call(L,2,LUA_MULTRET);
      lua_call(L,(lua_gettop(L) - lc32),LUA_MULTRET);
      lua_call(L,(lua_gettop(L) - lc31),1);
      lua_pushnumber(L,1);
      lc_add(L,3,-1);
      lua_remove(L,-2);
      lua_insert(L,-2);
      lua_settable(L,2);
      
      /* -- if the fourth char is not padding, we have a third byte
       * if d < 64 then */
      enum { lc33 = 8 };
      lua_pushnumber(L,64);
      const int lc34 = lua_lessthan(L,8,-1);
      lua_pop(L,1);
      lua_pushboolean(L,lc34);
      const int lc35 = lua_toboolean(L,-1);
      lua_pop(L,1);
      if (lc35) {
        
        /* -- Upper 2 bits come from Lower 2 bits of c
         * -- Lower 6 bits come from d
         * bytes[j + 2] = char(bor(lshift(band(c, 3), 6), d)) */
        lc_getupvalue(L,lua_upvalueindex(1),4,5);
        const int lc36 = lua_gettop(L);
        lc_getupvalue(L,lua_upvalueindex(1),6,3);
        lc_getupvalue(L,lua_upvalueindex(1),7,2);
        lc_getupvalue(L,lua_upvalueindex(1),5,4);
        lua_pushvalue(L,7);
        lua_pushnumber(L,3);
        lua_call(L,2,1);
        lua_pushnumber(L,6);
        lua_call(L,2,1);
        lua_pushvalue(L,8);
        lua_call(L,2,LUA_MULTRET);
        lua_call(L,(lua_gettop(L) - lc36),1);
        lua_pushnumber(L,2);
        lc_add(L,3,-1);
        lua_remove(L,-2);
        lua_insert(L,-2);
        lua_settable(L,2);
      }
      lua_settop(L,lc33);
    }
    lua_settop(L,lc28);
    
    /* j = j + 3 */
    lua_pushnumber(L,3);
    lc_add(L,3,-1);
    lua_remove(L,-2);
    lua_replace(L,3);
    
    /* internal: stack cleanup on scope exit */
    lua_pop(L,5);
    lc21_var += lc23_step;
  }
  lua_settop(L,lc25);
  
  /* return concat(bytes) */
  const int lc37 = lua_gettop(L);
  lc_getupvalue(L,lua_upvalueindex(1),2,7);
  lua_pushvalue(L,2);
  lua_call(L,1,LUA_MULTRET);
  return (lua_gettop(L) - lc37);
}


/* name: (main)
 * function(...) */
static int lcf_main (lua_State * L) {
  lua_checkstack(L,21);
  enum { lc_nformalargs = 0 };
  const int lc_nactualargs = lua_gettop(L);
  const int lc_nextra = (lc_nactualargs - lc_nformalargs);
  
  /* local rshift = bit32.rshift */
  lc_newclosuretable(L,lua_upvalueindex(1));
  enum { lc1 = 1 };
  lua_getfield(L,LUA_ENVIRONINDEX,"bit32");
  lua_pushliteral(L,"rshift");
  lua_gettable(L,-2);
  lua_remove(L,-2);
  lua_rawseti(L,(lc1 + lc_nextra),1);
  
  /* local lshift = bit32.lshift */
  lc_newclosuretable(L,(lc1 + lc_nextra));
  enum { lc2 = 2 };
  lua_getfield(L,LUA_ENVIRONINDEX,"bit32");
  lua_pushliteral(L,"lshift");
  lua_gettable(L,-2);
  lua_remove(L,-2);
  lua_rawseti(L,(lc2 + lc_nextra),2);
  
  /* local bor = bit32.bor */
  lc_newclosuretable(L,(lc2 + lc_nextra));
  enum { lc3 = 3 };
  lua_getfield(L,LUA_ENVIRONINDEX,"bit32");
  lua_pushliteral(L,"bor");
  lua_gettable(L,-2);
  lua_remove(L,-2);
  lua_rawseti(L,(lc3 + lc_nextra),3);
  
  /* local band = bit32.band */
  lc_newclosuretable(L,(lc3 + lc_nextra));
  enum { lc4 = 4 };
  lua_getfield(L,LUA_ENVIRONINDEX,"bit32");
  lua_pushliteral(L,"band");
  lua_gettable(L,-2);
  lua_remove(L,-2);
  lua_rawseti(L,(lc4 + lc_nextra),4);
  
  /* local char = string.char */
  lc_newclosuretable(L,(lc4 + lc_nextra));
  enum { lc5 = 5 };
  lua_getfield(L,LUA_ENVIRONINDEX,"string");
  lua_pushliteral(L,"char");
  lua_gettable(L,-2);
  lua_remove(L,-2);
  lua_rawseti(L,(lc5 + lc_nextra),5);
  
  /* local byte = string.byte */
  lc_newclosuretable(L,(lc5 + lc_nextra));
  enum { lc6 = 6 };
  lua_getfield(L,LUA_ENVIRONINDEX,"string");
  lua_pushliteral(L,"byte");
  lua_gettable(L,-2);
  lua_remove(L,-2);
  lua_rawseti(L,(lc6 + lc_nextra),6);
  
  /* local concat = table.concat */
  lc_newclosuretable(L,(lc6 + lc_nextra));
  enum { lc7 = 7 };
  lua_getfield(L,LUA_ENVIRONINDEX,"table");
  lua_pushliteral(L,"concat");
  lua_gettable(L,-2);
  lua_remove(L,-2);
  lua_rawseti(L,(lc7 + lc_nextra),7);
  
  /* local codes = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=' */
  lc_newclosuretable(L,(lc7 + lc_nextra));
  enum { lc8 = 8 };
  lua_pushliteral(L,"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=");
  lua_rawseti(L,(lc8 + lc_nextra),8);
  
  /* -- Loop over input 3 bytes at a time
   * -- a,b,c are 3 x 8-bit numbers
   * -- they are encoded into groups of 4 x 6-bit numbers
   * -- aaaaaa aabbbb bbbbcc cccccc
   * -- if there is no c, then pad the 4th with =
   * -- if there is also no b then pad the 3rd with =
   * local function base64Encode(str)
   *   local parts = {}
   *   local j = 1
   *   for i = 1, #str, 3 do
   *     local a, b, c = byte(str, i, i + 2)
   *     parts[j] = char(
   *       -- Higher 6 bits of a
   *       byte(codes, rshift(a, 2) + 1),
   *       -- Lower 2 bits of a + high 4 bits of b
   *       byte(codes, bor(
   *         lshift(band(a, 3), 4),
   *         b and rshift(b, 4) or 0
   *       ) + 1),
   *       -- Low 4 bits of b + High 2 bits of c
   *       b and byte(codes, bor(
   *         lshift(band(b, 15), 2),
   *         c and rshift(c, 6) or 0
   *       ) + 1) or 61, -- 61 is '='
   *       -- Lower 6 bits of c
   *       c and byte(codes, band(c, 63) + 1) or 61 -- 61 is '='
   *     )
   *     j = j + 1
   *   end
   *   return concat(parts)
   * end */
  lua_pushvalue(L,(lc8 + lc_nextra));
  lua_pushcclosure(L,lcf1_base64Encode, "encode", 1);
  
  /* -- Reverse map from character code to 6-bit integer
   * local map = {} */
  lc_newclosuretable(L,(lc8 + lc_nextra));
  enum { lc15 = 10 };
  lua_newtable(L);
  lua_rawseti(L,(lc15 + lc_nextra),9);
  
  /* for i = 1, #codes do */
  lua_pushnumber(L,1);
  lc_getupvalue(L,(lc15 + lc_nextra),1,8);
  const double lc19 = lua_objlen(L,-1);
  lua_pop(L,1);
  lua_pushnumber(L,lc19);
  if (!((lua_isnumber(L,-2) && lua_isnumber(L,-1)))) {
    luaL_error(L,"'for' limit must be a number");
  }
  double lc16_var = lua_tonumber(L,-2);
  const double lc17_limit = lua_tonumber(L,-1);
  const double lc18_step = 1;
  lua_pop(L,2);
  enum { lc20 = 10 };
  while ((((lc18_step > 0) && (lc16_var <= lc17_limit)) || ((lc18_step <= 0) && (lc16_var >= lc17_limit)))) {
    
    /* internal: local i at index 11 */
    lua_pushnumber(L,lc16_var);
    
    /* map[byte(codes, i)] = i - 1 */
    lua_pushnumber(L,1);
    lc_sub(L,(11 + lc_nextra),-1);
    lua_remove(L,-2);
    lc_getupvalue(L,(lc15 + lc_nextra),0,9);
    lua_insert(L,-2);
    lc_getupvalue(L,(lc15 + lc_nextra),3,6);
    lc_getupvalue(L,(lc15 + lc_nextra),1,8);
    lua_pushvalue(L,(11 + lc_nextra));
    lua_call(L,2,1);
    lua_insert(L,-2);
    lua_settable(L,-3);
    lua_pop(L,1);
    
    /* internal: stack cleanup on scope exit */
    lua_pop(L,1);
    lc16_var += lc18_step;
  }
  lua_settop(L,(lc20 + lc_nextra));
  
  /* -- loop over input 4 characters at a time
   * -- The characters are mapped to 4 x 6-bit integers a,b,c,d
   * -- They need to be reassalbled into 3 x 8-bit bytes
   * -- aaaaaabb bbbbcccc ccdddddd
   * -- if d is padding then there is no 3rd byte
   * -- if c is padding then there is no 2nd byte
   * local function base64Decode(data)
   *   local bytes = {}
   *   local j = 1
   *   for i = 1, #data, 4 do
   *     local a = map[byte(data, i)]
   *     local b = map[byte(data, i + 1)]
   *     local c = map[byte(data, i + 2)]
   *     local d = map[byte(data, i + 3)]
   * 
   *     -- higher 6 bits are the first char
   *     -- lower 2 bits are upper 2 bits of second char
   *     bytes[j] = char(bor(lshift(a, 2), rshift(b, 4)))
   * 
   *     -- if the third char is not padding, we have a second byte
   *     if c < 64 then
   *       -- high 4 bits come from lower 4 bits in b
   *       -- low 4 bits come from high 4 bits in c
   *       bytes[j + 1] = char(bor(lshift(band(b, 0xf), 4), rshift(c, 2)))
   * 
   *       -- if the fourth char is not padding, we have a third byte
   *       if d < 64 then
   *         -- Upper 2 bits come from Lower 2 bits of c
   *         -- Lower 6 bits come from d
   *         bytes[j + 2] = char(bor(lshift(band(c, 3), 6), d))
   *       end
   *     end
   *     j = j + 3
   *   end
   *   return concat(bytes)
   * end */
  lua_pushvalue(L,(lc15 + lc_nextra));
  lua_pushcclosure(L,lcf1_base64Decode, "decode", 1);
  
  /* return {
   *   encode = base64Encode,
   *   decode = base64Decode,
   * } */
  lc_register(L, LUA_BASE64LIBNAME, 2);
  lua_pushliteral(L,"encode");
  lua_pushvalue(L,(9 + lc_nextra));
  lua_rawset(L,-3);
  lua_pushliteral(L,"decode");
  lua_pushvalue(L,(11 + lc_nextra));
  lua_rawset(L,-3);
  return 1;
}

LUALIB_API int luaopen_base64(lua_State* L)
{
  lcf_main(L);

  return 1;
}