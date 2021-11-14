// This file is part of the Luau programming language and is licensed under MIT License; see LICENSE.txt for details
// This code is based on Lua 5.x implementation licensed under MIT License; see lua_LICENSE.txt for details
#include "lualib.h"

#include "lstate.h"
#include "lnumutils.h"

#include <math.h>
#include <time.h>

#undef LUA_MAXINTEGER
#define LUA_MAXINTEGER (LLONG_MAX)
#undef LUA_MININTEGER
#define LUA_MININTEGER (LLONG_MIN)

#undef EPSILON
#define EPSILON (4.94065645841247E-324)

#undef EPSILONF
#define EPSILONF (1.401298E-45F)

#undef PI
#define PI (3.14159265358979323846)
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
    int n = lua_tointegerx(L, 1, &valid);
    if (valid)
        lua_pushinteger(L, n);
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

static int math_eps(lua_State* L)
{
    double a = luaL_checknumber(L, 1);
    double b = luaL_checknumber(L, 2);
    double e = luaL_optnumber(L, 3, 1e-5);
	double aa = fabs(a) + 1;
    if (isinf(aa))
        lua_pushnumber(L, e);
    else
        lua_pushnumber(L, e * aa);
    return 1;
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

static int math_sqrt(lua_State* L)
{
    lua_pushnumber(L, sqrt(luaL_checknumber(L, 1)));
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
	lua_pushboolean(L, (fabs(luaL_checknumber(L, 1) - luaL_checknumber(L, 2)) < 1e-6));
    return 1;
}

static int math_ult (lua_State *L) {
    int a = luaL_checkinteger(L, 1);
    int b = luaL_checkinteger(L, 2);
    lua_pushboolean(L, (unsigned int)a < (unsigned int)b);
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
    double hash = luaL_checknumber(L, 1);
    double x = luaL_checknumber(L, 2);
    double y = luaL_checknumber(L, 3);
    double z = luaL_checknumber(L, 4);

    double r = grad((unsigned char)(int(hash)), (float)x, (float)y, (float)z);

    lua_pushnumber(L, r);

    return 1;
}

static int math_fade(lua_State* L)
{
    double t = luaL_checknumber(L, 1);

    double r = fade((float)t);

    lua_pushnumber(L, r);

    return 1;
}

static int math_lerp(lua_State* L)
{
    double t = luaL_checknumber(L, 1);
    double a = luaL_checknumber(L, 2);
    double b = luaL_checknumber(L, 3);

    double r = lerp((float)t, (float)a, (float)b);

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

static const luaL_Reg mathlib[] = {
    {"abs", math_abs},
    {"acos", math_acos},
    {"asin", math_asin},
    {"atan2", math_atan2},
    {"atan", math_atan},
    {"approximately", math_approximately},
    {"cbrt", math_cbrt},
    {"ceil", math_ceil},
    {"classify", math_classify},
    {"copysign", math_copysign},
    {"cosh", math_cosh},
    {"cos", math_cos},
    {"deg", math_deg},
    {"eps", math_eps},
    {"erf", math_erf},
    {"erfc", math_erfc},
    {"exp", math_exp},
    {"fade", math_fade},
    {"fdim", math_fdim},
    {"floor", math_floor},
    {"fma", math_fma},
    {"fmod", math_fmod},
    {"frexp", math_frexp},
    {"grad", math_grad},
    {"hypot", math_hypot},
    {"ilogb", math_ilogb},
    {"isinf", math_isinf},
    {"isfinite", math_isfinite},
    {"isnormal", math_isnormal},
    {"isnan", math_isnan},
    {"isunordered", math_isunordered},
    {"ldexp", math_ldexp},
    {"lerp", math_lerp},
    {"lgamma", math_lgamma},
    {"log10", math_log10},
    {"log1p", math_log1p},
    {"log", math_log},
    {"logb", math_logb},
    {"log2", math_log2},
    {"max", math_max},
    {"min", math_min},
    {"modf", math_modf},
    {"nexttoward", math_nexttoward},
    {"pow", math_pow},
    {"rad", math_rad},
    {"random", math_random},
    {"randomseed", math_randomseed},
    {"remainder", math_remainder},
    {"remquo", math_remquo},
    {"rep", math_rep},
    {"root", math_root},
    {"scalbn", math_scalbn},
    {"sinh", math_sinh},
    {"sin", math_sin},
    {"sqrt", math_sqrt},
    {"tanh", math_tanh},
    {"tan", math_tan},
    {"tgamma", math_tgamma},
    {"tointeger", math_toint},
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
    lua_pushnumber(L, NAN);
    lua_setfield(L, -2, "nan");
    lua_pushllong(L, LUA_MAXINTEGER);
    lua_setfield(L, -2, "maxinteger");
    lua_pushllong(L, LUA_MININTEGER);
    lua_setfield(L, -2, "mininteger");
    return 1;
}
