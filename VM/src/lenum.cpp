// This file is part of the Luau programming language and is licensed under MIT License; see LICENSE.txt for details
// This code is based on Lua 5.x implementation licensed under MIT License; see lua_LICENSE.txt for details
#include "lenum.h"

#include "lualib.h"
#include "lobject.h"
#include "lapi.h"
#include "lstate.h"

#include <string>
#include <map>

/**
 * Simple class for enum values
 */
class DynEnum {
  public:
    DynEnum(std::map<std::string, int> tbl) : tbl(tbl) {}

    int of(lua_State* L,std::string tag) {
        if (tbl.count(tag) > 0) {
            return tbl[tag];
        }
        luaL_error(L, "Enum tag not found");
    }

    std::string lookup(lua_State* L,int v) {
        for (auto &p : tbl) {
            if (p.second == v) {
                return p.first;
            }
        }
        luaL_error(L, "Enum value not found");
    }

  private:
    std::map<std::string, int> tbl;
};