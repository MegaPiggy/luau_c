// This file is part of the Luau programming language and is licensed under MIT License; see LICENSE.txt for details
// This code is based on Lua 5.x implementation licensed under MIT License; see lua_LICENSE.txt for details
#include "lualib.h"

#include "lstate.h"
#include "lnumutils.h"
#include "lc.h"

#include <math.h>
#include <time.h>

#include <map>

#include <boost/multiprecision/cpp_int.hpp>
#include "lboost.h"

#undef LUA_MAXINTEGER
#define LUA_MAXINTEGER (LLONG_MAX)
#undef LUA_MININTEGER
#define LUA_MININTEGER (LLONG_MIN)

#define MOD (1000003LL)

#undef EPSILON
#define EPSILON (4.94065645841247E-324)

#undef EPSILONF
#define EPSILONF (1.401298E-45)

#undef FUZZY_EPSILON
#define FUZZY_EPSILON (1e-30)

#undef FUZZY_EPSILONF
#define FUZZY_EPSILONF (1e-6)

#undef SQRT_FIVE
#define SQRT_FIVE sqrt(5)

#undef GOLDEN_RATIO
#define GOLDEN_RATIO ((1 + SQRT_FIVE) / 2)

#undef PI
#define PI (3.141592653589793238462643383279502884)
#define RADIANS_PER_DEGREE (PI / 180.0)

#define PCG32_INC 105

static uint32_t pcg32_random(uint64_t* state)
{
    uint64_t oldstate = *state;
    *state = oldstate * 6364136223846793005ULL + (PCG32_INC | 1);
    uint32_t xorshifted = uint32_t(((oldstate >> 18u) ^ oldstate) >> 27u);
    uint32_t rot = uint32_t(oldstate >> 59u);
    return (xorshifted >> rot) | (xorshifted << ((-int32_t(rot)) & 31));
}

static void pcg32_seed(uint64_t* state, uint64_t seed)
{
    *state = 0;
    pcg32_random(state);
    *state += seed;
    pcg32_random(state);
}

static int math_abs(lua_State* L)
{
    lua_pushnumber(L, fabs(luaL_checknumber(L, 1)));
    return 1;
}

static int math_sin(lua_State* L)
{
    lua_pushnumber(L, sin(luaL_checknumber(L, 1)));
    return 1;
}

static int math_sinh(lua_State* L)
{
    lua_pushnumber(L, sinh(luaL_checknumber(L, 1)));
    return 1;
}

static int math_cos(lua_State* L)
{
    lua_pushnumber(L, cos(luaL_checknumber(L, 1)));
    return 1;
}

static int math_cosh(lua_State* L)
{
    lua_pushnumber(L, cosh(luaL_checknumber(L, 1)));
    return 1;
}

static int math_tan(lua_State* L)
{
    lua_pushnumber(L, tan(luaL_checknumber(L, 1)));
    return 1;
}

static int math_tanh(lua_State* L)
{
    lua_pushnumber(L, tanh(luaL_checknumber(L, 1)));
    return 1;
}

static int math_asin(lua_State* L)
{
    lua_pushnumber(L, asin(luaL_checknumber(L, 1)));
    return 1;
}

static int math_acos(lua_State* L)
{
    lua_pushnumber(L, acos(luaL_checknumber(L, 1)));
    return 1;
}

static int math_atan(lua_State* L)
{
    lua_pushnumber(L, atan(luaL_checknumber(L, 1)));
    return 1;
}

static int math_atan2(lua_State* L)
{
    lua_pushnumber(L, atan2(luaL_checknumber(L, 1), luaL_checknumber(L, 2)));
    return 1;
}

static int math_toint(lua_State* L)
{
    int valid;
    boost::multiprecision::int1024_t n = lua_toint1024x(L, 1, &valid);
    if (valid)
        lua_pushint1024(L, n);
    else {
        luaL_checkany(L, 1);
        lua_pushnil(L);  /* value is not convertible to integer */
    }
    return 1;
}

static int math_ceil(lua_State* L)
{
    lua_pushnumber(L, ceil(luaL_checknumber(L, 1)));
    return 1;
}

static int math_floor(lua_State* L)
{
    lua_pushnumber(L, floor(luaL_checknumber(L, 1)));
    return 1;
}

static int math_rep(lua_State* L)
{
    double t = luaL_checknumber(L, 1);
    double length = luaL_checknumber(L, 2);
    lua_pushnumber(L, (t - (floor(t / length) * length)));
    return 1;
}

static double eps(double a, double b, double e)
{
	double aa = fabs(a) + 1;
    if (isinf(aa))
        return e;
    else
        return e * aa;
}

static int math_eps(lua_State* L)
{
    double a = luaL_checknumber(L, 1);
    double b = luaL_checknumber(L, 2);
    double e = luaL_optnumber(L, 3, FUZZY_EPSILONF);
    lua_pushnumber(L, eps(a, b, e));
    return 1;
}

static int math_fuzzyEq(lua_State* L)
{
    double a = luaL_checknumber(L, 1);
    double b = luaL_checknumber(L, 2);
    double e = luaL_optnumber(L, 3, FUZZY_EPSILONF);
    lua_pushboolean(L, (a == b || fabs(a - b) <= eps(a, b, e)));
    return 1;
}

static int math_fuzzyNe(lua_State* L)
{
    double a = luaL_checknumber(L, 1);
    double b = luaL_checknumber(L, 2);
    double e = luaL_optnumber(L, 3, FUZZY_EPSILONF);
    lua_pushboolean(L, (a == b || fabs(a - b) <= eps(a, b, e)) == 0);
    return 1;
}
 
static int math_fuzzyGt(lua_State* L)
{
    double a = luaL_checknumber(L, 1);
    double b = luaL_checknumber(L, 2);
    double e = luaL_optnumber(L, 3, FUZZY_EPSILONF);
    lua_pushboolean(L, (a > b + eps(a, b, e)));
    return 1;
}
 
static int math_fuzzyGe(lua_State* L)
{
    double a = luaL_checknumber(L, 1);
    double b = luaL_checknumber(L, 2);
    double e = luaL_optnumber(L, 3, FUZZY_EPSILONF);
    lua_pushboolean(L, (a > b - eps(a, b, e)));
    return 1;
}
 
static int math_fuzzyLt(lua_State* L)
{
    double a = luaL_checknumber(L, 1);
    double b = luaL_checknumber(L, 2);
    double e = luaL_optnumber(L, 3, FUZZY_EPSILONF);
    lua_pushboolean(L, (a < b - eps(a, b, e)));
    return 1;
}
 
static int math_fuzzyLe(lua_State* L)
{
    double a = luaL_checknumber(L, 1);
    double b = luaL_checknumber(L, 2);
    double e = luaL_optnumber(L, 3, FUZZY_EPSILONF);
    lua_pushboolean(L, (a < b + eps(a, b, e)));
    return 1;
}
 
double exponentMod(double a, double b, double m)
{
    // Base cases
    if (a == 0)
        return 0;
    if (b == 0)
        return 1;
 
    // If B is even
    double y;
    if (fmod(b, 2) == 0)
    {
        y = exponentMod(a, b / 2, m);
        y = fmod((y * y), m);
    }
 
    // If B is odd
    else
    {
        y = fmod(a, m);
        y = fmod((y * fmod(exponentMod(a, b - 1, m), m)), m);
    }
 
    return fmod((y + m), m);
}

static int math_powmod(lua_State* L)
{
    lua_pushnumber(L, exponentMod(luaL_checknumber(L, 1),luaL_checknumber(L, 2),luaL_checknumber(L, 3)));
    return 1;
}

static int math_invmod(lua_State* L)
{
    double a = luaL_checknumber(L, 1);
    double m = luaL_checknumber(L, 2);
    for (long long x = 1; x < m; x++)
    {
        if (fmod((fmod(a, m) * fmod(x, m)), m) == 1)
        {
            lua_pushint1024(L, x);
            return 1;
        }
    }
    return 0;
}

static int math_fmod(lua_State* L)
{
    lua_pushnumber(L, fmod(luaL_checknumber(L, 1), luaL_checknumber(L, 2)));
    return 1;
}

static int math_modf(lua_State* L)
{
    double ip;
    double fp = modf(luaL_checknumber(L, 1), &ip);
    lua_pushnumber(L, ip);
    lua_pushnumber(L, fp);
    return 2;
}

static int math_sqr(lua_State* L)
{
    double x = luaL_checknumber(L, 1);
    lua_pushnumber(L, x * x);
    return 1;
}

static int math_sqrt(lua_State* L)
{
    lua_pushnumber(L, sqrt(luaL_checknumber(L, 1)));
    return 1;
}

static int math_invsqrt(lua_State* L)
{
    lua_pushnumber(L, 1.0 / sqrt(luaL_checknumber(L, 1)));
    return 1;
}

static int math_cbrt(lua_State* L)
{
    lua_pushnumber(L, cbrt(luaL_checknumber(L, 1)));
    return 1;
}

static int math_root(lua_State* L)
{
    double n = luaL_checknumber(L, 1);
    double base = luaL_checknumber(L, 2);
    if (base < 1 || luai_nummod(n, 2) == 0 && n < 0)
        lua_pushnumber(L, NAN);
    else
        lua_pushnumber(L, pow(n, (1 / base)));
    return 1;
}

static int math_approximately(lua_State* L)
{
	lua_pushboolean(L, (fabs(luaL_checknumber(L, 1) - luaL_checknumber(L, 2)) < FUZZY_EPSILONF));
    return 1;
}

static int math_ult (lua_State *L) {
    boost::multiprecision::int1024_t a = luaL_checkint1024(L, 1);
    boost::multiprecision::int1024_t b = luaL_checkint1024(L, 2);
    lua_pushboolean(L, (boost::multiprecision::uint1024_t)a < (boost::multiprecision::uint1024_t)b);
    return 1;
}

static int math_pow(lua_State* L)
{
    lua_pushnumber(L, pow(luaL_checknumber(L, 1), luaL_checknumber(L, 2)));
    return 1;
}

static int math_log(lua_State* L)
{
    double x = luaL_checknumber(L, 1);
    double res;
    if (lua_isnoneornil(L, 2))
        res = log(x);
    else
    {
        double base = luaL_checknumber(L, 2);
        if (base == 2.0)
            res = log2(x);
        else if (base == 10.0)
            res = log10(x);
        else
            res = log(x) / log(base);
    }
    lua_pushnumber(L, res);
    return 1;
}

static int math_log10(lua_State* L)
{
    lua_pushnumber(L, log10(luaL_checknumber(L, 1)));
    return 1;
}

static int math_log1p(lua_State* L)
{
    lua_pushnumber(L, log1p(luaL_checknumber(L, 1)));
    return 1;
}

static int math_log2(lua_State* L)
{
    lua_pushnumber(L, log2(luaL_checknumber(L, 1)));
    return 1;
}

static int math_exp(lua_State* L)
{
    lua_pushnumber(L, exp(luaL_checknumber(L, 1)));
    return 1;
}

static int math_exp2(lua_State* L)
{
    lua_pushnumber(L, exp2(luaL_checknumber(L, 1)));
    return 1;
}

static int math_expm1(lua_State* L)
{
    lua_pushnumber(L, expm1(luaL_checknumber(L, 1)));
    return 1;
}

static int math_erf(lua_State* L)
{
    lua_pushnumber(L, erf(luaL_checknumber(L, 1)));
    return 1;
}

static int math_erfc(lua_State* L)
{
    lua_pushnumber(L, erfc(luaL_checknumber(L, 1)));
    return 1;
}

static int math_deg(lua_State* L)
{
    lua_pushnumber(L, luaL_checknumber(L, 1) / RADIANS_PER_DEGREE);
    return 1;
}

static int math_rad(lua_State* L)
{
    lua_pushnumber(L, luaL_checknumber(L, 1) * RADIANS_PER_DEGREE);
    return 1;
}

static int math_frexp(lua_State* L)
{
    int e;
    lua_pushnumber(L, frexp(luaL_checknumber(L, 1), &e));
    lua_pushinteger(L, e);
    return 2;
}

static int math_hypot(lua_State* L)
{
    lua_pushnumber(L, hypot(luaL_checknumber(L, 1), luaL_checknumber(L, 2)));
    return 1;
}

static int math_ilogb(lua_State* L)
{
    lua_pushinteger(L, ilogb(luaL_checknumber(L, 1)));
    return 1;
}

static int math_logb(lua_State* L)
{
    lua_pushnumber(L, logb(luaL_checknumber(L, 1)));
    return 1;
}

static int math_ldexp(lua_State* L)
{
    lua_pushnumber(L, ldexp(luaL_checknumber(L, 1), luaL_checkinteger(L, 2)));
    return 1;
}

static int math_remainder(lua_State* L)
{
    lua_pushnumber(L, remainder(luaL_checknumber(L, 1), luaL_checknumber(L, 2)));
    return 1;
}

static int math_remquo(lua_State* L)
{
    int quot;
    lua_pushnumber(L, remquo(luaL_checknumber(L, 1), luaL_checknumber(L, 2), &quot));
    lua_pushinteger(L, quot);
    return 2;
}

static int math_min(lua_State* L)
{
    int n = lua_gettop(L); /* number of arguments */
    double dmin = luaL_checknumber(L, 1);
    int i;
    for (i = 2; i <= n; i++)
    {
        double d = luaL_checknumber(L, i);
        if (d < dmin)
            dmin = d;
    }
    lua_pushnumber(L, dmin);
    return 1;
}

static int math_max(lua_State* L)
{
    int n = lua_gettop(L); /* number of arguments */
    double dmax = luaL_checknumber(L, 1);
    int i;
    for (i = 2; i <= n; i++)
    {
        double d = luaL_checknumber(L, i);
        if (d > dmax)
            dmax = d;
    }
    lua_pushnumber(L, dmax);
    return 1;
}

static int math_nearbyint(lua_State* L)
{
    lua_pushnumber(L, nearbyint(luaL_checknumber(L, 1)));
    return 1;
}

static int math_random(lua_State* L)
{
    global_State* g = L->global;
    switch (lua_gettop(L))
    { /* check number of arguments */
    case 0:
    { /* no arguments */
        // Using ldexp instead of division for speed & clarity.
        // See http://mumble.net/~campbell/tmp/random_real.c for details on generating doubles from integer ranges.
        uint32_t rl = pcg32_random(&g->rngstate);
        uint32_t rh = pcg32_random(&g->rngstate);
        double rd = ldexp(double(rl | (uint64_t(rh) << 32)), -64);
        lua_pushnumber(L, rd); /* number between 0 and 1 */
        break;
    }
    case 1:
    { /* only upper limit */
        int u = luaL_checkinteger(L, 1);
        luaL_argcheck(L, 1 <= u, 1, "interval is empty");

        uint64_t x = uint64_t(u) * pcg32_random(&g->rngstate);
        int r = int(1 + (x >> 32));
        lua_pushinteger(L, r); /* int between 1 and `u' */
        break;
    }
    case 2:
    { /* lower and upper limits */
        int l = luaL_checkinteger(L, 1);
        int u = luaL_checkinteger(L, 2);
        luaL_argcheck(L, l <= u, 2, "interval is empty");

        uint32_t ul = uint32_t(u) - uint32_t(l);
        luaL_argcheck(L, ul < UINT_MAX, 2, "interval is too large"); // -INT_MIN..INT_MAX interval can result in integer overflow
        uint64_t x = uint64_t(ul + 1) * pcg32_random(&g->rngstate);
        int r = int(l + (x >> 32));
        lua_pushinteger(L, r); /* int between `l' and `u' */
        break;
    }
    default:
        luaL_error(L, "wrong number of arguments");
    }
    return 1;
}

static int math_randomseed(lua_State* L)
{
    int seed = luaL_checkinteger(L, 1);

    pcg32_seed(&L->global->rngstate, seed);
    return 0;
}


static int math_type (lua_State *L) {
  if (lua_type(L, 1) == LUA_TNUMBER) {
      double n = lua_tonumber(L, 1);
      if (n == (int)n)
        lua_pushliteral(L, "integer");
      else
        lua_pushliteral(L, "float");
  }
  else {
    luaL_checkany(L, 1);
    lua_pushnil(L);
  }
  return 1;
}

static const unsigned char kPerlin[512] = {151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99,
    37, 240, 21, 10, 23, 190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174,
    20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41,
    55, 46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196, 135, 130, 116, 188, 159, 86,
    164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17,
    182, 189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9, 129, 22, 39, 253, 19, 98, 108, 110,
    79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14,
    239, 107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93, 222, 114, 67, 29, 24,
    72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180,

    151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247,
    120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74,
    165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54, 65,
    25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52,
    217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213,
    119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112,
    104, 218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199,
    106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61,
    156, 180};

static float fade(float t)
{
    return t * t * t * (t * (t * 6 - 15) + 10);
}

static float lerp(float t, float a, float b)
{
    return a + t * (b - a);
}

static float grad(unsigned char hash, float x, float y, float z)
{
    unsigned char h = hash & 15;
    float u = (h < 8) ? x : y;
    float v = (h < 4) ? y : (h == 12 || h == 14) ? x : z;

    return (h & 1 ? -u : u) + (h & 2 ? -v : v);
}

static float perlin(float x, float y, float z)
{
    float xflr = floorf(x);
    float yflr = floorf(y);
    float zflr = floorf(z);

    int xi = int(xflr) & 255;
    int yi = int(yflr) & 255;
    int zi = int(zflr) & 255;

    float xf = x - xflr;
    float yf = y - yflr;
    float zf = z - zflr;

    float u = fade(xf);
    float v = fade(yf);
    float w = fade(zf);

    const unsigned char* p = kPerlin;

    int a = p[xi] + yi;
    int aa = p[a] + zi;
    int ab = p[a + 1] + zi;

    int b = p[xi + 1] + yi;
    int ba = p[b] + zi;
    int bb = p[b + 1] + zi;

    return lerp(w,
        lerp(v, lerp(u, grad(p[aa], xf, yf, zf), grad(p[ba], xf - 1, yf, zf)), lerp(u, grad(p[ab], xf, yf - 1, zf), grad(p[bb], xf - 1, yf - 1, zf))),
        lerp(v, lerp(u, grad(p[aa + 1], xf, yf, zf - 1), grad(p[ba + 1], xf - 1, yf, zf - 1)),
            lerp(u, grad(p[ab + 1], xf, yf - 1, zf - 1), grad(p[bb + 1], xf - 1, yf - 1, zf - 1))));
}

static int math_grad(lua_State* L)
{
    unsigned char hash = (unsigned char)luaL_checkinteger(L, 1);
    double x = luaL_checknumber(L, 2);
    double y = luaL_checknumber(L, 3);
    double z = luaL_checknumber(L, 4);

    unsigned char h = hash & 15;
    double u = (h < 8) ? x : y;
    double v = (h < 4) ? y : (h == 12 || h == 14) ? x : z;
    double r = (h & 1 ? -u : u) + (h & 2 ? -v : v);

    lua_pushnumber(L, r);

    return 1;
}

static int math_fade(lua_State* L)
{
    double t = luaL_checknumber(L, 1);

    double r = t * t * t * (t * (t * 6 - 15) + 10);

    lua_pushnumber(L, r);

    return 1;
}

static int math_lerp(lua_State* L)
{
    double t = luaL_checknumber(L, 1);
    double a = luaL_checknumber(L, 2);
    double b = luaL_checknumber(L, 3);
    double precise = luaL_optboolean(L, 4, false);

    double r = precise ? ((1 - t) * a + t * b) : (a + t * (b - a));

    lua_pushnumber(L, r);

    return 1;
}

static int math_noise(lua_State* L)
{
    double x = luaL_checknumber(L, 1);
    double y = luaL_optnumber(L, 2, 0.0);
    double z = luaL_optnumber(L, 3, 0.0);

    double r = perlin((float)x, (float)y, (float)z);

    lua_pushnumber(L, r);

    return 1;
}

static int math_clamp(lua_State* L)
{
    double v = luaL_checknumber(L, 1);
    double min = luaL_checknumber(L, 2);
    double max = luaL_checknumber(L, 3);

    luaL_argcheck(L, min <= max, 3, "max must be greater than or equal to min");

    double r = v < min ? min : v;
    r = r > max ? max : r;

    lua_pushnumber(L, r);
    return 1;
}

static int math_sign(lua_State* L)
{
    double v = luaL_checknumber(L, 1);
    lua_pushnumber(L, v > 0.0 ? 1.0 : v < 0.0 ? -1.0 : 0.0);
    return 1;
}

static int math_signbit(lua_State* L)
{
    lua_pushinteger(L, signbit(luaL_checknumber(L, 1)));
    return 1;
}

static int math_copysign(lua_State* L)
{
    lua_pushnumber(L, copysign(luaL_checknumber(L, 1), luaL_checknumber(L, 2)));
    return 1;
}

static int math_round(lua_State* L)
{
    double v = luaL_checknumber(L, 1);
    lua_pushnumber(L, round(v));
    return 1;
}

static int math_scalbn(lua_State* L)
{
    lua_pushnumber(L, scalbn(luaL_checknumber(L, 1), luaL_checkinteger(L, 2)));
    return 1;
}

static int math_tgamma(lua_State* L)
{
    lua_pushnumber(L, tgamma(luaL_checknumber(L, 1)));
    return 1;
}

static int math_lgamma(lua_State* L)
{
    lua_pushnumber(L, lgamma(luaL_checknumber(L, 1)));
    return 1;
}

static int math_trunc(lua_State* L)
{
    lua_pushnumber(L, trunc(luaL_checknumber(L, 1)));
    return 1;
}

static int math_fdim(lua_State* L)
{
    lua_pushnumber(L, fdim(luaL_checknumber(L, 1), luaL_checknumber(L, 2)));
    return 1;
}

static int math_fma(lua_State* L)
{
    lua_pushnumber(L, fma(luaL_checknumber(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3)));
    return 1;
}

static int math_nextafter(lua_State* L)
{
    lua_pushnumber(L, nextafter(luaL_checknumber(L, 1), luaL_checknumber(L, 2)));
    return 1;
}

static int math_nexttoward(lua_State* L)
{
    lua_pushnumber(L, nexttoward(luaL_checknumber(L, 1), luaL_checknumber(L, 2)));
    return 1;
}

static int math_classify(lua_State* L)
{
    switch (fpclassify(luaL_checknumber(L, 1))) {
        case FP_INFINITE:
            lua_pushliteral(L, "infinite");
            break;
        case FP_NAN:
            lua_pushliteral(L, "NaN");
            break;
        case FP_ZERO:
            lua_pushliteral(L, "zero");
            break;
        case FP_SUBNORMAL:
            lua_pushliteral(L, "subnormal");
            break;
        case FP_NORMAL:
            lua_pushliteral(L, "normal");
            break;
        default:
            lua_pushnil(L);
            break;
    }
    return 1;
}

#ifdef MISSING_ISINF
#define isinf(x) (!isnan(x) && isnan((x) - (x)))
#endif

static int math_isinf(lua_State* L)
{
    lua_pushboolean(L, isinf(luaL_checknumber(L, 1)));
    return 1;
}

static int math_isfinite(lua_State* L)
{
    lua_pushboolean(L, isfinite(luaL_checknumber(L, 1)));
    return 1;
}

static int math_isnormal(lua_State* L)
{
    lua_pushboolean(L, isnormal(luaL_checknumber(L, 1)));
    return 1;
}

static int math_isnan(lua_State* L)
{
    lua_pushboolean(L, isnan(luaL_checknumber(L, 1)));
    return 1;
}

static int math_isunordered(lua_State* L)
{
    lua_pushboolean(L, isunordered(luaL_checknumber(L, 1), luaL_checknumber(L, 2)));
    return 1;
}

static int math_iseven(lua_State* L)
{
    lua_pushboolean(L, luaL_checkint1024(L, 1) % 2 == 0);
    return 1;
}

static int math_isodd(lua_State* L)
{
    lua_pushboolean(L, luaL_checkint1024(L, 1) % 2 != 0);
    return 1;
}

static int math_isprime(lua_State* L)
{
    boost::multiprecision::int1024_t n = luaL_checkint1024(L, 1);
    // 0 and 1 are not prime numbers
    if (n == 0 || n == 1) {
        lua_pushboolean(L, false);
        return 1;
    }
    else {
        for (boost::multiprecision::int1024_t i = 2; i <= n / 2; ++i) {
            if (n % i == 0) {
                lua_pushboolean(L, false);
                return 1;
            }
        }
    }
    lua_pushboolean(L, true);
    return 1;
}

// Function to find reverse of any number
static int math_reverse(lua_State* L)
{
    boost::multiprecision::int1024_t number = luaL_checkint1024(L, 1);
    boost::multiprecision::int1024_t reversedNumber = 0;
    while (number != 0)
    {
        boost::multiprecision::int1024_t remainder = number % 10;
        reversedNumber = reversedNumber * 10 + remainder;
        number /= 10;
    }
    lua_pushint1024(L, reversedNumber);
    return 1;
}

// Function to calculate order of the number
static int math_ispalindrome(lua_State* L)
{
    boost::multiprecision::int1024_t number = luaL_checkint1024(L, 1);
    boost::multiprecision::int1024_t reversedNumber = 0;
    boost::multiprecision::int1024_t originalNumber = number;
    while (number != 0)
    {
        boost::multiprecision::int1024_t remainder = number % 10;
        reversedNumber = reversedNumber * 10 + remainder;
        number /= 10;
    }
    lua_pushboolean(L, originalNumber == reversedNumber);
    return 1;
}

// Function to find n'th carol number
static int math_carol(lua_State* L)
{
    int64_t n = luaL_checkllong(L, 1);
    int64_t an = abs(n);
    boost::multiprecision::cpp_int result = boost::multiprecision::pow(boost::multiprecision::cpp_int(2), an) - 1;
    result = result * result - 2;
    boost::multiprecision::int1024_t result1024 = (boost::multiprecision::int1024_t)result;
    if (an != n)
        lua_pushint1024(L, -result1024);
    else
        lua_pushint1024(L, result1024);
    return 1;
}

static boost::multiprecision::uint1024_t permute(boost::multiprecision::uint1024_t n, boost::multiprecision::uint1024_t k)
{
    boost::multiprecision::uint1024_t result = 1;

    for (; n > k; --n)
        result *= n;

    return result;
}

static int math_permute(lua_State* L)
{
    boost::multiprecision::uint1024_t n = luaL_checkuint1024(L, 1);
    boost::multiprecision::uint1024_t k = luaL_checkuint1024(L, 2);
    lua_pushuint1024(L, permute(n, k));
    return 1;
}

static inline boost::multiprecision::uint1024_t binomial(boost::multiprecision::uint1024_t n, boost::multiprecision::uint1024_t k)
{
    if ( n - k > k )
        return permute(n, n - k) / permute(k, 1);
    else
        return permute(n, k) / permute(n - k, 1);
}

static int math_binomial(lua_State* L)
{
    boost::multiprecision::uint1024_t n = luaL_checkuint1024(L, 1);
    boost::multiprecision::uint1024_t k = luaL_checkuint1024(L, 2);
    lua_pushuint1024(L, binomial(n, k));
    return 1;
}

static int math_bell(lua_State* L)
{
  const int limit = lua_tointeger(L,1)+1;
  // Base case
  if (limit <= 1){
    lua_pushinteger(L, 1);
    return 1;
  }
  else if (limit > 219){
    lua_pushnumber(L, HUGE_VAL);
    return 1;
  }

  lua_createtable(L,1,0);
  lua_createtable(L,1,0);
  lua_pushinteger(L,1);
  lua_rawseti(L,-2,1);
  lua_rawseti(L,-2,1);
  
  int i = 2;
  const int step = 1;
  while ((((step > 0) && (i <= limit)) || ((step <= 0) && (i >= limit))))
  {
    lua_pushinteger(L,i);
    
    lua_createtable(L,1,0);
    lua_pushinteger(L,1);
    lc_sub(L,3,-1);
    lua_remove(L,-2);
    lua_gettable(L,2);
    lua_pushinteger(L,1);
    lc_sub(L,3,-1);
    lua_remove(L,-2);
    lua_gettable(L,-2);
    lua_remove(L,-2);
    lua_rawseti(L,-2,1);
    lua_pushvalue(L,3);
    lua_insert(L,-2);
    lua_settable(L,2);
    
    int i2 = 2;
    const int limit2 = lua_tointeger(L,3);
    const int step2 = 1;
    while ((((step2 > 0) && (i2 <= limit2)) || ((step2 <= 0) && (i2 >= limit2))))
    {
      lua_pushinteger(L,i2);
      
      lua_pushvalue(L,3);
      lua_gettable(L,2);
      lua_pushinteger(L,1);
      lc_sub(L,4,-1);
      lua_remove(L,-2);
      lua_gettable(L,-2);
      lua_remove(L,-2);
      lua_pushinteger(L,1);
      lc_sub(L,3,-1);
      lua_remove(L,-2);
      lua_gettable(L,2);
      lua_pushinteger(L,1);
      lc_sub(L,4,-1);
      lua_remove(L,-2);
      lua_gettable(L,-2);
      lua_remove(L,-2);
      lc_add(L,-2,-1);
      lua_remove(L,-2);
      lua_remove(L,-2);
      lua_pushvalue(L,3);
      lua_gettable(L,2);
      lua_insert(L,-2);
      lua_pushvalue(L,4);
      lua_insert(L,-2);
      lua_settable(L,-3);
      lua_pop(L,1);
      
      lua_pop(L,1);
      i2 += step2;
    }
    lua_settop(L,3);
    
    lua_pop(L,1);
    i += step;
  }
  lua_settop(L,2);
  
  lua_rawgeti(L,2,limit);
  lua_rawgeti(L,3, 1);
  return 1;
}

// Function to calculate x raised to the power y
boost::multiprecision::int1024_t power(boost::multiprecision::int1024_t x, boost::multiprecision::uint1024_t y)
{
    if (y == 0)
        return 1;
    if (y % 2 == 0)
        return power(x, y / 2) * power(x, y / 2);
    return x * power(x, y / 2) * power(x, y / 2);
}

// Function to calculate order of the number
int order(boost::multiprecision::int1024_t x)
{
    int n = 0;
    while (x)
    {
        n++;
        x = x / 10;
    }
    return n;
}

// Function to check whether the given number is Armstrong number or not
static int math_isarmstrong(lua_State* L)
{
    boost::multiprecision::int1024_t x = luaL_checkint1024(L, 1);
    // Calling order function
    int n = order(x);
    boost::multiprecision::int1024_t temp = x, sum = 0;
    while (temp)
    {
        boost::multiprecision::int1024_t r = temp % 10;
        sum += power(r, n);
        temp = temp / 10;
    }

    // Satisfies Armstrong condition
    lua_pushboolean(L, sum == x);
    return 1;
}

static int math_fib(lua_State* L)
{
    boost::multiprecision::int1024_t n = luaL_checkint1024(L, 1);
    boost::multiprecision::int1024_t an = boost::multiprecision::abs(n);
    if (an > 1474)
    {
        lua_pushnumber(L, HUGE_VAL); 
        return 1;
    }
    bool m = an > n;
    double numerator = pow(GOLDEN_RATIO, (double)n) - pow(1-GOLDEN_RATIO, (double)n);
    double denominator = SQRT_FIVE;
    // This cast should in general work, as the result is always an integer. 
    // Floating point errors may occur!
    // if (an >= 93)
    // {
    //     if (m)
    //     {
    //         luaL_argerror(L, 1, "cannot be below -92");
    //         return 0;
    //     }
    //     else
    //     {
    //         if (an > 93)
    //         {
    //             luaL_argerror(L, 1, "cannot be above 93");
    //             return 0;
    //         }
    //         else
    //             lua_pushullong(L, static_cast<unsigned long long>(numerator/denominator)); 
    //     }
    // }
    // else
    //     lua_pushllong(L, static_cast<long long>(numerator/denominator)); 
    lua_pushint1024(L, boost::multiprecision::int1024_t(numerator/denominator)); 
    return 1;
}

// Function to check whether the given number is a factorial of some number or not
static int math_isfact(lua_State* L)
{
    boost::multiprecision::uint1024_t n = luaL_checkuint1024(L, 1);
    if (n <= 0) {
        lua_pushboolean(L, false);
        return 1;
    }
    for (boost::multiprecision::uint1024_t i = 1;; i++) {
        if (n % i != 0)
            break;
        n = n / i;
    }
    
    lua_pushboolean(L, n == 1);
    return 1;
}

/** function to find factorial of given number */
boost::multiprecision::uint1024_t factorial(boost::multiprecision::uint1024_t n, boost::multiprecision::uint1024_t step = 1) {
    boost::multiprecision::uint1024_t res = 1;
    for (boost::multiprecision::uint1024_t i = n;; i -= step) {
        if (i == 0 || i == 1)
            return res;
        res *= i;
    }
    return res;
}

static int math_fact(lua_State* L)
{
    boost::multiprecision::uint1024_t n = luaL_checkuint1024(L, 1);
    if (n < 1026)
    {
        lua_pushuint1024(L, factorial(n, luaL_optuint1024(L, 2, 1))); 
        return 1;
    }
    else
    {
        lua_pushnumber(L, HUGE_VAL); 
        return 1;
    }
}

// Returns value of Binomial Coefficient C(n, k)
static int binomialCoeff(int n, int k)
{
    // Base Cases
    if (k > n)
        return 0;
    if (k == 0 || k == n)
        return 1;
  
    // Recur
    return binomialCoeff(n - 1, k - 1)
           + binomialCoeff(n - 1, k);
}

static std::map<long long, unsigned long long> catalanMap = std::map<long long, unsigned long long>
{
    {2, 2},
    {3, 5},
    {4, 14},
    {5, 42},
    {6, 132},
    {7, 429},
    {8, 1430},
    {9, 4862},
    {10, 16796},
    {11, 58786},
    {12, 208012},
    {13, 742900},
    {14, 2674440},
    {15, 9694845},
    {16, 35357670},
    {17, 129644790},
    {18, 477638700},
    {19, 1767263190},
    {20, 6564120420},
    {21, 24466267020},
    {22, 91482563640},
    {23, 343059613650},
    {24, 1289904147324},
    {25, 4861946401452},
    {26, 18367353072152},
    {27, 69533550916004},
    {28, 263747951750360},
    {29, 1002242216651368},
    {30, 3814986502092304},
    {31, 14544636039226909},
    {32, 55534064877048198},
    {33, 212336130412243110},
    {34, 812944042149730764},
    {35, 3116285494907301262},
    {36, 11959798385860453492},
};

static bool getCatalan(long long key, unsigned long long& out)
{
    std::map<long long, unsigned long long>::iterator it = catalanMap.find(key);
    if (it != catalanMap.end())
    {
        out = it->second;
        return true;
    }
    return false;
}

// Function to find n'th catalan number
static int math_catalan(lua_State* L)
{
    long long n = luaL_checkllong(L, 1);

    // Base case
    if (n <= 1)
    {
        lua_pushinteger(L, 1);
        return 1;
    }
 
    unsigned long long out;
    if (getCatalan(n, out))
    {
        lua_pushullong(L, out);
        return 1;
    }
    else
    {
        luaL_error(L, "number is too big to calculate");
        return 0;
    }
}

/**
 * Function to calculate the sum of all the proper divisor
 * of an integer.
 * @param num First number.
 * @return Sum of the proper divisor of the number.
 */
boost::multiprecision::int1024_t sum_of_divisor(boost::multiprecision::int1024_t num) {
    // Variable to store the sum of all proper divisors.
    boost::multiprecision::int1024_t sum = 0;
    // Below loop condition helps to reduce Time complexity by a factor of
    // square root of the number.
    for (boost::multiprecision::int1024_t div = 2; div * div <= num; ++div) {
        // Check 'div' is divisor of 'num'.
        if (num % div == 0) {
            // If both divisor are same, add once to 'sum'
            if (div == (num / div)) {
                sum += div;
            } else {
                // If both divisor are not the same, add both to 'sum'.
                sum += (div + (num / div));
            }
        }
    }
    return sum + 1;
}

// Function to calculate the sum of all the proper divisor
static int math_sumdivisor(lua_State* L)
{
    lua_pushint1024(L, sum_of_divisor(luaL_checkint1024(L, 1)));
    return 1;
}

// Function to check whether the pair is amicable or not.
static int math_amicable(lua_State* L)
{
    boost::multiprecision::int1024_t x = luaL_checkint1024(L, 1);
    boost::multiprecision::int1024_t y = luaL_checkint1024(L, 2);
    lua_pushboolean(L, (sum_of_divisor(x) == y) && (sum_of_divisor(y) == x));
    return 1;
}

// Recursive function to return gcd of a and b
static double gcd(double a, double b)
{
    if (a < b)
        return gcd(b, a);
 
    // base case
    if (b < 0.001)
        return a;
    else
        return (gcd(b, a - floor(a / b) * b));
}

static int math_gcd(lua_State* L)
{
    int n = lua_gettop(L); /* number of arguments */
    if (n == 1)
    {
        double a = fabs(luaL_checknumber(L, 1));
        lua_pushnumber(L, gcd(a,a));
    }
    else if (n == 2)
    {
        double a = fabs(luaL_checknumber(L, 1));
        double b = fabs(luaL_checknumber(L, 2));
        lua_pushnumber(L, gcd(a,b));
    }
    else
    {
        double result = fabs(luaL_checknumber(L, 1));
        for (int i = 2; i <= n; i++)
        {
            double a = fabs(luaL_checknumber(L, i));
            result = gcd(a, result);
    
            if(result == 1)
            {
                lua_pushinteger(L, 1);
                return 1;
            }
        }
        lua_pushnumber(L, result);
    }
    return 1;
}

// Function to return LCM of two numbers
static double lcm(double a, double b)
{
    return (a / gcd(a, b)) * b;
}

static int math_lcm(lua_State* L)
{
    int n = lua_gettop(L); /* number of arguments */
    if (n == 1)
    {
        double a = fabs(luaL_checknumber(L, 1));
        lua_pushnumber(L, lcm(a,a));
    }
    else if (n == 2)
    {
        double a = fabs(luaL_checknumber(L, 1));
        double b = fabs(luaL_checknumber(L, 2));
        lua_pushnumber(L, lcm(a,b));
    }
    else
    {
        double ans = fabs(luaL_checknumber(L, 1));
        for (int i = 2; i <= n; i++){
            double a = fabs(luaL_checknumber(L, i));
            ans = (((a * ans)) / (gcd(a, ans)));
        }
        lua_pushnumber(L, ans);
    }
    return 1;
}

static int math_mean(lua_State* L)
{
    int n = lua_gettop(L); /* number of arguments */
    double sum = 0;

    for (int i = 1; i <= n; i++)
    {
        double a = luaL_checknumber(L, i);
        sum = sum + a;
    }

    lua_pushnumber(L, sum / (double)n);
    return 1;
}

#include <vector>
#include <algorithm>    // std::sort

static int math_median(lua_State* L)
{
    int n = lua_gettop(L); /* number of arguments */
    int sum = 0;

    std::vector<double> args = {};    

    for (int i = 1; i <= n; i++)
    {
        double a = luaL_checknumber(L, i);
        args.push_back(a);
    }

    std::sort(args.begin(), args.end());

    if (n % 2 == 0)
        lua_pushnumber(L, (args[n / 2] + args[(n / 2) - 1]) / 2);
    else
        lua_pushnumber(L, args[n / 2]);

    return 1;
}

static int math_tostring(lua_State* L)
{
    double val = luaL_checknumber(L, 1);
    
    // Check for NaN, -inf and inf
    if (isnan(val)){
        lua_pushliteral(L, "NaN");
        return 1;
    }
    else if (isinf(val)){
        if (val > 0.0)
            lua_pushliteral(L, "inf");
        else
            lua_pushliteral(L, "-inf");
        return 1;
    }

    luaL_Buffer b;
    luaL_buffinit(L, &b);
    char buff[512];   /* to store the formatted item */
    sprintf(buff, "%.f", val);
    luaL_addlstring(&b, buff, strlen(buff));
    luaL_pushresult(&b);
    
    return 1;
}

static int math_compare(lua_State* L)
{
    double a = luaL_checknumber(L, 1);
    double b = luaL_checknumber(L, 2);
    if (a == b)
        lua_pushinteger(L, 0);
    else if (a > b)
        lua_pushinteger(L, 1);
    else
        lua_pushinteger(L, -1);
    return 1;
}

static const luaL_Reg mathlib[] = {
    {"abs", math_abs},
    {"acos", math_acos},
    {"asin", math_asin},
    {"atan2", math_atan2},
    {"atan", math_atan},
    {"amicable", math_amicable},
    {"approximately", math_approximately},
    {"bell", math_bell},
    {"binomial", math_binomial},
    {"carol", math_carol},
    {"catalan", math_catalan},
    {"cbrt", math_cbrt},
    {"ceil", math_ceil},
    {"classify", math_classify},
    {"compare", math_compare},
    {"copysign", math_copysign},
    {"cosh", math_cosh},
    {"cos", math_cos},
    {"deg", math_deg},
    {"eps", math_eps},
    {"erf", math_erf},
    {"erfc", math_erfc},
    {"exp", math_exp},
    {"exp2", math_exp2},
    {"expm1", math_expm1},
    {"fade", math_fade},
    {"fdim", math_fdim},
    {"fib", math_fib},
    {"fact", math_fact},
    {"floor", math_floor},
    {"fma", math_fma},
    {"fmod", math_fmod},
    {"fuzzyeq", math_fuzzyEq},
    {"fuzzyge", math_fuzzyGe},
    {"fuzzygt", math_fuzzyGt},
    {"fuzzyle", math_fuzzyLe},
    {"fuzzylt", math_fuzzyLt},
    {"fuzzyne", math_fuzzyNe},
    {"frexp", math_frexp},
    {"grad", math_grad},
    {"gcd", math_gcd},
    {"hcf", math_gcd},
    {"hypot", math_hypot},
    {"invmod", math_invmod},
    {"invsqrt", math_invsqrt},
    {"ilogb", math_ilogb},
    {"isarmstrong", math_isarmstrong},
    {"iseven", math_iseven},
    {"isinf", math_isinf},
    {"isfact", math_isfact},
    {"isfinite", math_isfinite},
    {"isnormal", math_isnormal},
    {"isnan", math_isnan},
    {"isodd", math_isodd},
    {"ispalindrome", math_ispalindrome},
    {"isprime", math_isprime},
    {"isunordered", math_isunordered},
    {"lcm", math_lcm},
    {"ldexp", math_ldexp},
    {"lerp", math_lerp},
    {"lgamma", math_lgamma},
    {"log10", math_log10},
    {"log1p", math_log1p},
    {"log", math_log},
    {"logb", math_logb},
    {"log2", math_log2},
    {"max", math_max},
    {"mean", math_mean},
    {"median", math_median},
    {"min", math_min},
    {"modf", math_modf},
    {"nearbyint", math_nearbyint},
    {"nextafter", math_nextafter},
    {"nexttoward", math_nexttoward},
    {"permute", math_permute},
    {"pow", math_pow},
    {"powmod", math_powmod},
    {"rad", math_rad},
    {"random", math_random},
    {"randomseed", math_randomseed},
    {"remainder", math_remainder},
    {"remquo", math_remquo},
    {"rep", math_rep},
    {"reverse", math_reverse},
    {"root", math_root},
    {"scalbn", math_scalbn},
    {"sinh", math_sinh},
    {"sin", math_sin},
    {"sqr", math_sqr},
    {"sqrt", math_sqrt},
    {"sumdivisor", math_sumdivisor},
    {"tanh", math_tanh},
    {"tan", math_tan},
    {"tgamma", math_tgamma},
    {"tointeger", math_toint},
    {"tostring", math_tostring},
    {"trunc", math_trunc},
    {"type", math_type},
    {"ult",   math_ult},
    {"noise", math_noise},
    {"clamp", math_clamp},
    {"sign", math_sign},
    {"signbit", math_signbit},
    {"round", math_round},
    {NULL, NULL},
};

/*
** Open math library
*/
LUALIB_API int luaopen_math(lua_State* L)
{
    uint64_t seed = uintptr_t(L);
    seed ^= time(NULL);
    seed ^= clock();

    pcg32_seed(&L->global->rngstate, seed);

    luaL_register(L, LUA_MATHLIBNAME, mathlib);
    lua_pushnumber(L, PI);
    lua_setfield(L, -2, "pi");
    lua_pushnumber(L, HUGE_VAL);
    lua_setfield(L, -2, "huge");
    lua_pushnumber(L, EPSILON);
    lua_setfield(L, -2, "epsilon");
    lua_pushnumber(L, EPSILONF);
    lua_setfield(L, -2, "epsilonf");
    lua_pushnumber(L, FUZZY_EPSILON);
    lua_setfield(L, -2, "fuzzyepsilon");
    lua_pushnumber(L, FUZZY_EPSILONF);
    lua_setfield(L, -2, "fuzzyepsilonf");
    lua_pushnumber(L, NAN);
    lua_setfield(L, -2, "nan");
    lua_pushllong(L, LUA_MAXINTEGER);
    lua_setfield(L, -2, "maxinteger");
    lua_pushllong(L, LUA_MININTEGER);
    lua_setfield(L, -2, "mininteger");
    lua_pushnumber(L, GOLDEN_RATIO);
    lua_setfield(L, -2, "goldenratio");
    lua_pushnumber(L, MOD);
    lua_setfield(L, -2, "mod");
    return 1;
}
