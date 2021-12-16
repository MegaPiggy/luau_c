#include "lualib.h"
#include "lapi.h"
#include "lstate.h"
#include <string>
#include <map>

/* macro to `unsign' a character */
#define uchar(c) ((unsigned char)(c))

/* macro to `unsign' the first character in a string */
#define ustring(c) ((unsigned char)((c)[0]))

#ifdef MISSING_ISINF
#define isinf(x) (!isnan(x) && isnan((x) - (x)))
#endif

static std::map<unsigned char, const char *> escape_char_map = std::map<unsigned char, const char *>{
  {ustring("\\"), "\\"},
  {ustring("\""), "\""},
  {ustring("\b"), "b"},
  {ustring("\f"), "f"},
  {ustring("\n"), "n"},
  {ustring("\r"), "r"},
  {ustring("\t"), "t"},
  {ustring("\u0000"), "u0000"},
  {ustring("\u0001"), "u0001"},
  {ustring("\u0002"), "u0002"},
  {ustring("\u0003"), "u0003"},
  {ustring("\u0004"), "u0004"},
  {ustring("\u0005"), "u0005"},
  {ustring("\u0006"), "u0006"},
  {ustring("\u0007"), "u0007"},
  {ustring("\u000b"), "u000b"},
  {ustring("\u000e"), "u000e"},
  {ustring("\u000f"), "u000f"},
  {ustring("\u0010"), "u0010"},
  {ustring("\u0011"), "u0011"},
  {ustring("\u0012"), "u0012"},
  {ustring("\u0013"), "u0013"},
  {ustring("\u0014"), "u0014"},
  {ustring("\u0015"), "u0015"},
  {ustring("\u0016"), "u0016"},
  {ustring("\u0017"), "u0017"},
  {ustring("\u0018"), "u0018"},
  {ustring("\u0019"), "u0019"},
  {ustring("\u001a"), "u001a"},
  {ustring("\u001b"), "u001b"},
  {ustring("\u001c"), "u001c"},
  {ustring("\u001d"), "u001d"},
  {ustring("\u001e"), "u001e"},
  {ustring("\u001f"), "u001f"},
  {ustring("\u007f"), "u007f"},
};

static std::map<const char *, unsigned char> escape_char_map_inv = std::map<const char *, unsigned char>{ {"/", ustring("/")} };


std::string escape_char(unsigned char c)
{
    return addTwoStrings("\\", escape_char_map[c]); // or string.format("u%04x", c:byte())
}

std::string addTwoStrings(const std::string& a, const std::string& b)
{
    return a + b; // works because they are both strings.
}

static int encode_nil(lua_State* L)
{
    lua_pushliteral(L, "null");
    return 1;
}

static int encode_table(lua_State* L)
{
    int top = lua_gettop(L);
    printf("%i", top);
    if (lua_type(L, 2) != LUA_TTABLE)
        lua_createtable(L, 0, 0); // stack
    lua_createtable(L, 0, 0); // result
    return 1;
}

static int encode_string(lua_State* L)
{
    const char* val = lua_tostring(L, 1);
    lua_pushstring(L, addTwoStrings("\"", addTwoStrings(val, "\"")).c_str());
    return 1;
}

static int encode_number(lua_State* L)
{
    double val = lua_tonumber(L, 1);
    
    // Check for NaN, -inf and inf
    if (isnan(val)){
        luaL_argerrorL(L, 1, "unexpected number value 'NaN'");
        return 0;
    }
    else if (isinf(val)){
        if (val > 0.0)
            luaL_argerrorL(L, 1, "unexpected number value 'inf'");
        else
            luaL_argerrorL(L, 1, "unexpected number value '-inf'");
        return 0;
    }

    luaL_Buffer b;
    luaL_buffinit(L, &b);
    char buff[512];   /* to store the formatted item */
    sprintf(buff, "%.f", val);
    luaL_addlstring(&b, buff, strlen(buff));
    luaL_pushresult(&b);
    
    return 1;
}

static int encode_boolean(lua_State* L)
{
    if (lua_toboolean(L, 1))
        lua_pushliteral(L, "true");
    else
        lua_pushliteral(L, "false");
        
    return 1;
}

static int json_encode(lua_State* L)
{
    int type = lua_type(L, 1);
    switch (type)
    {
        case LUA_TNONE:
        case LUA_TNIL:
            return encode_nil(L);
        case LUA_TBOOLEAN:
            return encode_boolean(L);
        case LUA_TNUMBER:
            return encode_number(L);
        case LUA_TSTRING:
            return encode_string(L);
        case LUA_TTABLE:
            return encode_table(L);
        default:
            luaL_argerrorL(L, 1, addTwoStrings("unexpected type '", addTwoStrings(luaL_typename(L, 1), "'")).c_str());
            return 0;
    }
}

static int json_decode(lua_State* L)
{
    size_t l;
    const char* val = luaL_checklstring(L, 1, &l);
    return 0;
}

static const luaL_Reg jsonlib[] = {
    {"encode", json_encode},
    {"decode", json_decode},
    {NULL, NULL},
};

/*
** Open json library
*/
LUALIB_API int luaopen_json(lua_State* L)
{
    luaL_register(L, LUA_JSONLIBNAME, jsonlib);

    std::map<unsigned char, const char*>::iterator it = escape_char_map.begin();
    while (it != escape_char_map.end())
    {
        unsigned char key = it->first;
        const char* value = it->second;
        escape_char_map_inv.insert(std::pair<const char*, unsigned char>(value, key));
        // Increment the Iterator to point to next entry
        it++;
    }

    return 1;
}
