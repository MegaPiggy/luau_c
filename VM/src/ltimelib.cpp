#include "lualib.h"
#include "lapi.h"
#include "lstate.h"
#include "lboost.h"

#include <time.h>
#include <chrono>

typedef std::chrono::high_resolution_clock Clock;
#define tonanoseconds(v) (((std::chrono::nanoseconds)std::chrono::duration_cast<std::chrono::nanoseconds>(v.time_since_epoch())));
#define compare(start, end) ((std::chrono::nanoseconds)std::chrono::duration_cast<std::chrono::nanoseconds>(end - start));


static int time_now(lua_State *L)
{
    std::chrono::nanoseconds now = tonanoseconds(Clock::now());
    lua_pushint1024(L, boost::multiprecision::int1024_t(now.count()));
    return 1;
}

static const luaL_Reg timelib[] = {
    {"now", time_now},
    {NULL, NULL},
};

/*
** Open time library
*/
LUALIB_API int luaopen_time(lua_State* L)
{
    luaL_register(L, LUA_TIMELIBNAME, timelib);

    return 1;
}
