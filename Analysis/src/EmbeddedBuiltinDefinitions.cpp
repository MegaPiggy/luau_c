// This file is part of the Luau programming language and is licensed under MIT License; see LICENSE.txt for details
#include "Luau/BuiltinDefinitions.h"

namespace Luau
{

static const std::string kBuiltinDefinitionLuaSrc = R"BUILTIN_SRC(

declare bit32: {
    band: (...number) -> number,
    bor: (...number) -> number,
    bxor: (...number) -> number,
    btest: (number, ...number) -> boolean,
    rrotate: (number, number) -> number,
    lrotate: (number, number) -> number,
    lshift: (number, number) -> number,
    arshift: (number, number) -> number,
    rshift: (number, number) -> number,
    bnot: (number) -> number,
    extract: (number, number, number?) -> number,
    replace: (number, number, number, number?) -> number,
    countlz: (number) -> number,
    countrz: (number) -> number,
}

declare math: {
    frexp: (number) -> (number, number),
    ldexp: (number, number) -> number,
    fmod: (number, number) -> number,
    modf: (number) -> (number, number),
    pow: (number, number) -> number,
    exp: (number) -> number,

    ceil: (number) -> number,
    floor: (number) -> number,
    abs: (number) -> number,
    sqrt: (number) -> number,

    log: (number, number?) -> number,
    log10: (number) -> number,

    rad: (number) -> number,
    deg: (number) -> number,

    sin: (number) -> number,
    cos: (number) -> number,
    tan: (number) -> number,
    sinh: (number) -> number,
    cosh: (number) -> number,
    tanh: (number) -> number,
    atan: (number) -> number,
    acos: (number) -> number,
    asin: (number) -> number,
    atan2: (number, number) -> number,

    min: (number, ...number) -> number,
    max: (number, ...number) -> number,

    pi: number,
    tau: number,
    huge: number,

    randomseed: (number) -> (),
    random: (number?, number?) -> number,

    sign: (number) -> number,
    clamp: (number, number, number) -> number,
    noise: (number, number?, number?) -> number,
    round: (number) -> number,

    epsilon: number,
    epsilonf: number,
    nan: number,
    maxinteger: number,
    mininteger: number,
    fuzzyepsilon: number,
    fuzzyepsilonf: number,

    approximately: (number) -> boolean,
    cbrt: (number) -> number,
    classify: (number) -> string,
    copysign: (number, number) -> number,
    eps: (number, number, number?) -> number,
    erf: (number) -> number,
    erfc: (number) -> number,
    exp2: (number) -> number,
    expm1: (number) -> number,
    fade: (number) -> number,
    fdim: (number, number) -> number,
    fma: (number, number, number) -> number,
    fuzzyeq: (number, number, number?) -> boolean,
    fuzzyne: (number, number, number?) -> boolean,
    fuzzygt: (number, number, number?) -> boolean,
    fuzzyge: (number, number, number?) -> boolean,
    fuzzylt: (number, number, number?) -> boolean,
    fuzzyle: (number, number, number?) -> boolean,
    grad: (number, number, number, number) -> number,
    hypot: (number, number) -> number,
    ilogb: (number) -> number,
    isinf: (number) -> boolean,
    isfinite: (number) -> boolean,
    isnormal: (number) -> boolean,
    isnan: (number) -> boolean,
    isunordered: (number) -> boolean,
    lerp: (number, number, number) -> number,
    lgamma: (number) -> number,
    log1p: (number) -> number,
    logb: (number) -> number,
    log2: (number) -> number,
    nexttoward: (number, number) -> number,
    remainder: (number, number) -> number,
    remquo: (number, number) -> (number, number),
    rep: (number, number) -> number,
    root: (number, number) -> number,
    scalbn: (number, number) -> number,
    signbit: (number) -> number,
    tgamma: (number) -> number,
    tointeger: <T>(value: T) -> number?,
    trunc: (number) -> number,
    type: (number) -> string,
    ult: (number, number) -> boolean,
}

type DateTypeArg = {
    year: number,
    month: number,
    day: number,
    hour: number?,
    min: number?,
    sec: number?,
    isdst: boolean?,
}

type DateTypeResult = {
    year: number,
    month: number,
    wday: number,
    yday: number,
    day: number,
    hour: number,
    min: number,
    sec: number,
    isdst: boolean,
}

declare os: {
    time: (DateTypeArg?) -> number,
    date: (string?, number?) -> DateTypeResult | string,
    difftime: (DateTypeResult | number, DateTypeResult | number) -> number,
    clock: () -> number,
}

declare function require(target: any): any

declare function getfenv(target: any): { [string]: any }

declare _G: any
declare _VERSION: string

declare function gcinfo(): number

declare function print<T...>(...: T...)

declare function type<T>(value: T): string
declare function typeof<T>(value: T): string

-- `assert` has a magic function attached that will give more detailed type information
declare function assert<T>(value: T, errorMessage: string?): T

declare function error<T>(message: T, level: number?)

declare function tostring<T>(value: T): string
declare function tonumber<T>(value: T, radix: number?): number?

declare function rawequal<T1, T2>(a: T1, b: T2): boolean
declare function rawget<K, V>(tab: {[K]: V}, k: K): V
declare function rawset<K, V>(tab: {[K]: V}, k: K, v: V): {[K]: V}

declare function setfenv<T..., R...>(target: number | (T...) -> R..., env: {[string]: any}): ((T...) -> R...)?

declare function ipairs<V>(tab: {V}): (({V}, number) -> (number, V), {V}, number)

declare function pcall<A..., R...>(f: (A...) -> R..., ...: A...): (boolean, R...)

-- FIXME: The actual type of `xpcall` is:
-- <E, A..., R1..., R2...>(f: (A...) -> R1..., err: (E) -> R2..., A...) -> (true, R1...) | (false, R2...)
-- Since we can't represent the return value, we use (boolean, R1...).
declare function xpcall<E, A..., R1..., R2...>(f: (A...) -> R1..., err: (E) -> R2..., ...: A...): (boolean, R1...)

-- `select` has a magic function attached to provide more detailed type information
declare function select<A...>(i: string | number, ...: A...): ...any

-- FIXME: This type is not entirely correct - `loadstring` returns a function or
-- (nil, string).
declare function loadstring<A...>(src: string, chunkname: string?): (((A...) -> any)?, string?)

declare function wait(seconds: number)

declare function newproxy(mt: boolean?): any

declare coroutine: {
    create: <A..., R...>((A...) -> R...) -> thread,
    resume: <A..., R...>(thread, A...) -> (boolean, R...),
    running: () -> thread,
    status: (thread) -> "dead" | "running" | "normal" | "suspended",
    -- FIXME: This technically returns a function, but we can't represent this yet.
    wrap: <A..., R...>((A...) -> R...) -> any,
    yield: <A..., R...>(A...) -> R...,
    isyieldable: () -> boolean,
    close: (thread) -> (boolean, any)
}

declare table: {
    concat: <V>({V}, string?, number?, number?) -> string,
    insert: (<V>({V}, V) -> ()) & (<V>({V}, number, V) -> ()),
    maxn: <V>({V}) -> number,
    remove: <V>({V}, number?) -> V?,
    sort: <V>({V}, ((V, V) -> boolean)?) -> (),
    create: <V>(number, V?) -> {V},
    find: <V>({V}, V, number?) -> number?,

    unpack: <V>({V}, number?, number?) -> ...V,
    pack: <V>(...V) -> { n: number, [number]: V },

    getn: <V>({V}) -> number,
    foreach: <K, V>({[K]: V}, (K, V) -> ()) -> (),
    foreachi: <V>({V}, (number, V) -> ()) -> (),

    move: <V>({V}, number, number, number, {V}?) -> {V},
    clear: <K, V>({[K]: V}) -> (),

    freeze: <K, V>({[K]: V}) -> {[K]: V},
    isfrozen: <K, V>({[K]: V}) -> boolean,

    isempty: <K, V>({[K]: V}) -> boolean,
    first: <K, V>({[K]: V}) -> V,
}

declare debug: {
    info: (<R...>(thread, number, string) -> R...) & (<R...>(number, string) -> R...) & (<A..., R1..., R2...>((A...) -> R1..., string) -> R2...),
    traceback: ((string?, number?) -> string) & ((thread, string?, number?) -> string),
}

type CprCustomOptions = {
    followRedirects: number?,
    maxRedirects: number?,
    timeout: number?,
    auth: {}?,
    digest: {}?,
    ntlm: {}?,
    bearer: string?,
    payload: {}?,
    multipart: {}?,
    parameters: {}?,
}

type CprResponseStatus = {
    line: string,
    code: number,
}

type CprResponseError = {
    message: string,
    code: string,
}

type CprResponse = {
    elapsed: number,
    text: string,
    reason: string,
    downloaded: number,
    uploaded: number,
    redirects: number,
    status: CprResponseStatus,
    error: CprResponseError,
    url: string,
    rawHeader: string,
    header: {},
    cookies: {},
    certInfo: {},
}

declare cpr: {
    request: (string, string, {}?, string?, CprCustomOptions?) -> CprResponse,
    get: (string, {}?, string?, CprCustomOptions?) -> CprResponse,
    post: (string, {}?, string?, CprCustomOptions?) -> CprResponse,
    patch: (string, {}?, string?, CprCustomOptions?) -> CprResponse,
    put: (string, {}?, string?, CprCustomOptions?) -> CprResponse,
    delete: (string, {}?, string?, CprCustomOptions?) -> CprResponse,
    options: (string, {}?, string?, CprCustomOptions?) -> CprResponse,
    head: (string, {}?, string?, CprCustomOptions?) -> CprResponse,
}

declare json: {
    encode: (any) -> string,
    decode: (string) -> any,
}

declare base64: {
    encode: ((string) -> string) & ((number) -> string),
    decode: (string) -> string,
}

declare utf8: {
    char: (...number) -> string,
    charpattern: string,
    codes: (string) -> ((string, number) -> (number, number), string, number),
    -- FIXME
    codepoint: (string, number?, number?) -> (number, ...number),
    len: (string, number?, number?) -> (number?, number?),
    offset: (string, number?, number?) -> number,
    nfdnormalize: (string) -> string,
    nfcnormalize: (string) -> string,
    graphemes: (string, number?, number?) -> (() -> (number, number)),
}

-- Cannot use `typeof` here because it will produce a polytype when we expect a monotype.
declare function unpack<V>(tab: {V}, i: number?, j: number?): ...V

)BUILTIN_SRC";

std::string getBuiltinDefinitionSource()
{
    return kBuiltinDefinitionLuaSrc;
}

} // namespace Luau
