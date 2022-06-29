// This file is part of the Luau programming language and is licensed under MIT License; see LICENSE.txt for details
#include "Builtins.h"

#include "Luau/Bytecode.h"
#include "Luau/Compiler.h"

namespace Luau
{
namespace Compile
{

Builtin getBuiltin(AstExpr* node, const DenseHashMap<AstName, Global>& globals, const DenseHashMap<AstLocal*, Variable>& variables)
{
    if (AstExprLocal* expr = node->as<AstExprLocal>())
    {
        const Variable* v = variables.find(expr->local);

        return v && !v->written && v->init ? getBuiltin(v->init, globals, variables) : Builtin();
    }
    else if (AstExprIndexName* expr = node->as<AstExprIndexName>())
    {
        if (AstExprGlobal* object = expr->expr->as<AstExprGlobal>())
        {
            return getGlobalState(globals, object->name) == Global::Default ? Builtin{object->name, expr->index} : Builtin();
        }
        else
        {
            return Builtin();
        }
    }
    else if (AstExprGlobal* expr = node->as<AstExprGlobal>())
    {
        return getGlobalState(globals, expr->name) == Global::Default ? Builtin{AstName(), expr->name} : Builtin();
    }
    else
    {
        return Builtin();
    }
}

int getBuiltinFunctionId(const Builtin& builtin, const CompileOptions& options)
{
    if (builtin.empty())
        return -1;

    if (builtin.isGlobal("assert"))
        return LBF_ASSERT;

    if (builtin.isGlobal("type"))
        return LBF_TYPE;

    if (builtin.isGlobal("typeof"))
        return LBF_TYPEOF;

    if (builtin.isGlobal("rawset"))
        return LBF_RAWSET;
    if (builtin.isGlobal("rawget"))
        return LBF_RAWGET;
    if (builtin.isGlobal("rawequal"))
        return LBF_RAWEQUAL;

    if (builtin.isGlobal("unpack"))
        return LBF_TABLE_UNPACK;

    if (builtin.isGlobal("select"))
        return LBF_SELECT_VARARG;

    if (builtin.isGlobal("wait"))
        return LBF_WAIT;

    if (builtin.object == "math")
    {
        if (builtin.method == "abs")
            return LBF_MATH_ABS;
        if (builtin.method == "acos")
            return LBF_MATH_ACOS;
        if (builtin.method == "asin")
            return LBF_MATH_ASIN;
        if (builtin.method == "atan2")
            return LBF_MATH_ATAN2;
        if (builtin.method == "atan")
            return LBF_MATH_ATAN;
        if (builtin.method == "ceil")
            return LBF_MATH_CEIL;
        if (builtin.method == "cosh")
            return LBF_MATH_COSH;
        if (builtin.method == "cos")
            return LBF_MATH_COS;
        if (builtin.method == "deg")
            return LBF_MATH_DEG;
        if (builtin.method == "exp")
            return LBF_MATH_EXP;
        if (builtin.method == "floor")
            return LBF_MATH_FLOOR;
        if (builtin.method == "fmod")
            return LBF_MATH_FMOD;
        if (builtin.method == "frexp")
            return LBF_MATH_FREXP;
        if (builtin.method == "ldexp")
            return LBF_MATH_LDEXP;
        if (builtin.method == "log10")
            return LBF_MATH_LOG10;
        if (builtin.method == "log")
            return LBF_MATH_LOG;
        if (builtin.method == "max")
            return LBF_MATH_MAX;
        if (builtin.method == "min")
            return LBF_MATH_MIN;
        if (builtin.method == "modf")
            return LBF_MATH_MODF;
        if (builtin.method == "pow")
            return LBF_MATH_POW;
        if (builtin.method == "rad")
            return LBF_MATH_RAD;
        if (builtin.method == "sinh")
            return LBF_MATH_SINH;
        if (builtin.method == "sin")
            return LBF_MATH_SIN;
        if (builtin.method == "sqrt")
            return LBF_MATH_SQRT;
        if (builtin.method == "tanh")
            return LBF_MATH_TANH;
        if (builtin.method == "tan")
            return LBF_MATH_TAN;
        if (builtin.method == "clamp")
            return LBF_MATH_CLAMP;
        if (builtin.method == "sign")
            return LBF_MATH_SIGN;
        if (builtin.method == "round")
            return LBF_MATH_ROUND;
        if (builtin.method == "approximately")
            return LBF_MATH_APPROXIMATELY;
        if (builtin.method == "cbrt")
            return LBF_MATH_CBRT;
        if (builtin.method == "classify")
            return LBF_MATH_CLASSIFY;
        if (builtin.method == "copysign")
            return LBF_MATH_COPYSIGN;
        if (builtin.method == "eps")
            return LBF_MATH_EPS;
        if (builtin.method == "erf")
            return LBF_MATH_ERF;
        if (builtin.method == "erfc")
            return LBF_MATH_ERFC;
        if (builtin.method == "exp2")
            return LBF_MATH_EXP2;
        if (builtin.method == "expm1")
            return LBF_MATH_EXPM1;
        if (builtin.method == "fade")
            return LBF_MATH_FADE;
        if (builtin.method == "fdim")
            return LBF_MATH_FDIM;
        if (builtin.method == "fma")
            return LBF_MATH_FMA;
        if (builtin.method == "fuzzyeq")
            return LBF_MATH_FUZZYEQ;
        if (builtin.method == "fuzzyne")
            return LBF_MATH_FUZZYNE;
        if (builtin.method == "fuzzygt")
            return LBF_MATH_FUZZYGT;
        if (builtin.method == "fuzzyge")
            return LBF_MATH_FUZZYGE;
        if (builtin.method == "fuzzylt")
            return LBF_MATH_FUZZYLT;
        if (builtin.method == "fuzzyle")
            return LBF_MATH_FUZZYLE;
        if (builtin.method == "grad")
            return LBF_MATH_GRAD;
        if (builtin.method == "hypot")
            return LBF_MATH_HYPOT;
        if (builtin.method == "ilogb")
            return LBF_MATH_ILOGB;
        if (builtin.method == "isinf")
            return LBF_MATH_ISINF;
        if (builtin.method == "isfinite")
            return LBF_MATH_ISFINITE;
        if (builtin.method == "isnormal")
            return LBF_MATH_ISNORMAL;
        if (builtin.method == "isnan")
            return LBF_MATH_ISNAN;
        if (builtin.method == "isunordered")
            return LBF_MATH_ISUNORDERED;
        if (builtin.method == "lerp")
            return LBF_MATH_LERP;
        if (builtin.method == "lgamma")
            return LBF_MATH_LGAMMA;
        if (builtin.method == "log1p")
            return LBF_MATH_LOG1P;
        if (builtin.method == "logb")
            return LBF_MATH_LOGB;
        if (builtin.method == "log2")
            return LBF_MATH_LOG2;
        if (builtin.method == "nexttoward")
            return LBF_MATH_NEXTTOWARD;
        if (builtin.method == "remainder")
            return LBF_MATH_REMAINDER;
        if (builtin.method == "remquo")
            return LBF_MATH_REMQUO;
        if (builtin.method == "rep")
            return LBF_MATH_REP;
        if (builtin.method == "root")
            return LBF_MATH_ROOT;
        if (builtin.method == "scalbn")
            return LBF_MATH_SCALBN;
        if (builtin.method == "signbit")
            return LBF_MATH_SIGNBIT;
        if (builtin.method == "tgamma")
            return LBF_MATH_TGAMMA;
        if (builtin.method == "tointeger")
            return LBF_MATH_TOINTEGER;
        if (builtin.method == "trunc")
            return LBF_MATH_TRUNC;
        if (builtin.method == "type")
            return LBF_MATH_TYPE;
        if (builtin.method == "ult")
            return LBF_MATH_ULT;
    }

    if (builtin.object == "bit32")
    {
        if (builtin.method == "arshift")
            return LBF_BIT32_ARSHIFT;
        if (builtin.method == "band")
            return LBF_BIT32_BAND;
        if (builtin.method == "bnot")
            return LBF_BIT32_BNOT;
        if (builtin.method == "bor")
            return LBF_BIT32_BOR;
        if (builtin.method == "bxor")
            return LBF_BIT32_BXOR;
        if (builtin.method == "btest")
            return LBF_BIT32_BTEST;
        if (builtin.method == "extract")
            return LBF_BIT32_EXTRACT;
        if (builtin.method == "lrotate")
            return LBF_BIT32_LROTATE;
        if (builtin.method == "lshift")
            return LBF_BIT32_LSHIFT;
        if (builtin.method == "replace")
            return LBF_BIT32_REPLACE;
        if (builtin.method == "rrotate")
            return LBF_BIT32_RROTATE;
        if (builtin.method == "rshift")
            return LBF_BIT32_RSHIFT;
        if (builtin.method == "countlz")
            return LBF_BIT32_COUNTLZ;
        if (builtin.method == "countrz")
            return LBF_BIT32_COUNTRZ;
    }

    if (builtin.object == "string")
    {
        if (builtin.method == "byte")
            return LBF_STRING_BYTE;
        if (builtin.method == "char")
            return LBF_STRING_CHAR;
        if (builtin.method == "len")
            return LBF_STRING_LEN;
        if (builtin.method == "sub")
            return LBF_STRING_SUB;
        if (builtin.method == "trim")
            return LBF_STRING_TRIM;
        if (builtin.method == "trimstart")
            return LBF_STRING_TRIM_START;
        if (builtin.method == "trimend")
            return LBF_STRING_TRIM_END;
        if (builtin.method == "index")
            return LBF_STRING_INDEX;
    }

    if (builtin.object == "table")
    {
        if (builtin.method == "insert")
            return LBF_TABLE_INSERT;
        if (builtin.method == "unpack")
            return LBF_TABLE_UNPACK;
        if (builtin.method == "isempty")
            return LBF_TABLE_ISEMPTY;
        if (builtin.method == "first")
            return LBF_TABLE_FIRST;
    }

    if (builtin.object == "cpr")
    {
        if (builtin.method == "request")
            return LBF_CPR_REQUEST;
        if (builtin.method == "get")
            return LBF_CPR_GET;
        if (builtin.method == "post")
            return LBF_CPR_POST;
        if (builtin.method == "patch")
            return LBF_CPR_PATCH;
            if (builtin.method == "put")
            return LBF_CPR_PUT;
        if (builtin.method == "delete")
            return LBF_CPR_DELETE;
        if (builtin.method == "options")
            return LBF_CPR_OPTIONS;
        if (builtin.method == "head")
            return LBF_CPR_HEAD;
    }

    if (builtin.object == "json")
    {
        if (builtin.method == "encode")
            return LBF_JSON_ENCODE;
        if (builtin.method == "decode")
            return LBF_JSON_DECODE;
    }

    if (builtin.object == "base64")
    {
        if (builtin.method == "encode")
            return LBF_BASE64_ENCODE;
        if (builtin.method == "decode")
            return LBF_BASE64_DECODE;
    }

    if (options.vectorCtor)
    {
        if (options.vectorLib)
        {
            if (builtin.isMethod(options.vectorLib, options.vectorCtor))
                return LBF_VECTOR;
        }
        else
        {
            if (builtin.isGlobal(options.vectorCtor))
                return LBF_VECTOR;
        }
    }

    return -1;
}

} // namespace Compile
} // namespace Luau
