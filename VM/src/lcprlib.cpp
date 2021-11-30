#include "lualib.h"

#include "lgc.h"
#include "lobject.h"
#include "lapi.h"
#include "ltable.h"
#include "lstate.h"
#include <cpr/cpr.h>

#undef strdup
#define strdup _strdup

#undef strlwr
#define strlwr _strlwr

#define CPR_RESPONSE "cprResponse"

typedef struct ResponseData
{
    //private:
        cpr::Response* object;
} ResponseData;

class Auth
{
    public:
        bool hasBasic = false;
        cpr::Authentication basic = cpr::Authentication("","");
        bool hasDigest = false;
        cpr::Digest digest = cpr::Digest("","");
        bool hasNTLM = false;
        cpr::NTLM ntlm = cpr::NTLM("","");
        bool hasBearer = false;
        cpr::Bearer bearer = cpr::Bearer("");

        Auth() {}; //Default constructor

        Auth(cpr::Authentication basicAuth)
         : basic(basicAuth)
         , hasBasic(true) {};

        Auth(cpr::Digest digestAuth)
         : digest(digestAuth)
         , hasDigest(true) {};

        Auth(cpr::NTLM ntlmAuth)
         : ntlm(ntlmAuth)
         , hasNTLM(true) {};

        Auth(cpr::Bearer bearerAuth)
         : bearer(bearerAuth)
         , hasBearer(true) {};

        void AddBasic(cpr::Authentication basicAuth){
            basic = basicAuth;
            hasBasic = true;
        };

        void AddDigest(cpr::Digest digestAuth){
            digest = digestAuth;
            hasDigest = true;
        };

        void AddNTLM(cpr::NTLM ntlmAuth){
            ntlm = ntlmAuth;
            hasNTLM = true;
        };

        void AddBearer(cpr::Bearer bearerAuth){
            bearer = bearerAuth;
            hasBearer = true;
        };
};


enum RequestMethod
{
    None,
    Get,
    Post,
    Patch,
    Put,
    Delete,
    Head,
    Options
};

static std::map<std::string, RequestMethod, cpr::CaseInsensitiveCompare> requestMap = std::map<std::string, RequestMethod, cpr::CaseInsensitiveCompare>{
    {"GET", RequestMethod::Get},
    {"get", RequestMethod::Get},
    {"Get", RequestMethod::Get},

    {"POST", RequestMethod::Post},
    {"post", RequestMethod::Post},
    {"Post", RequestMethod::Post},

    {"PATCH", RequestMethod::Patch},
    {"patch", RequestMethod::Patch},
    {"Patch", RequestMethod::Patch},

    {"PUT", RequestMethod::Put},
    {"put", RequestMethod::Put},
    {"Put", RequestMethod::Put},

    {"DELETE", RequestMethod::Delete},
    {"delete", RequestMethod::Delete},
    {"Delete", RequestMethod::Delete},

    {"HEAD", RequestMethod::Head},
    {"head", RequestMethod::Head},
    {"Head", RequestMethod::Head},

    {"OPTIONS", RequestMethod::Options},
    {"options", RequestMethod::Options},
    {"Options", RequestMethod::Options},
};

template <typename... Ts>
cpr::Response Request(RequestMethod method, Ts&&... ts) {
    switch( method )
    {
        case RequestMethod::Get:
            
            return (cpr::Response) cpr::Get(std::forward<Ts>(ts)...);
        case RequestMethod::Post:
            return (cpr::Response) cpr::Post(std::forward<Ts>(ts)...);
        case RequestMethod::Patch:
            return (cpr::Response) cpr::Patch(std::forward<Ts>(ts)...);
        case RequestMethod::Put:
            return (cpr::Response) cpr::Put(std::forward<Ts>(ts)...);
        case RequestMethod::Delete:
            return (cpr::Response) cpr::Delete(std::forward<Ts>(ts)...);
        case RequestMethod::Head:
            return (cpr::Response) cpr::Head(std::forward<Ts>(ts)...);
        case RequestMethod::Options:
            return (cpr::Response) cpr::Options(std::forward<Ts>(ts)...);
        case RequestMethod::None:
        default:
            return cpr::Response();
    }
}

static l_noret table_arg_error(lua_State* L, const char* nname, const char* extramsg)
{
    const char* fname = luaL_currfuncname(L);

    if (fname)
        luaL_error(L, "invalid %s to '%s' (%s)", nname, fname, extramsg);
    else
        luaL_error(L, "invalid %s (%s)", nname, extramsg);
}

static l_noret table_type_error(lua_State* L, int narg, const char* nname, const char* tname)
{
    const char* fname = luaL_currfuncname(L);
    const TValue* obj = luaA_toobject(L, narg);

    if (obj)
    {
        if (fname)
            luaL_error(L, "invalid %s to '%s' (%s expected, got %s)", nname, fname, tname, luaT_objtypename(L, obj));
        else
            luaL_error(L, "invalid %s (%s expected, got %s)", nname, tname, luaT_objtypename(L, obj));
    }
    else
    {
        if (fname)
            luaL_error(L, "missing %s to '%s' (%s expected)", nname, fname, tname);
        else
            luaL_error(L, "missing %s (%s expected)", nname, tname);
    }
}

static l_noret table_type_error(lua_State* L, int narg, const char* nname, int tag)
{
    table_type_error(L, narg, nname, lua_typename(L, tag));
}

static void checktablefortable(lua_State* L, int n, const char* nname)
{
    if (lua_type(L, n) != LUA_TTABLE)
        table_type_error(L, n, nname, LUA_TTABLE);
}

static const char* checktableforstring(lua_State* L, int n, const char* nname)
{
    const char* s = lua_tolstring(L, (n), NULL);
    if (!s)
        table_type_error(L, n, nname, LUA_TSTRING);
    return s;
}

static const char* opttableforstring(lua_State* L, int n, const char* nname, const char* def)
{
    if (lua_isnoneornil(L, n))
        return def;
    else
        return checktableforstring(L, n, nname);
}

static const char* checktableforlstring(lua_State* L, int n, const char* nname, size_t* len)
{
    const char* s = lua_tolstring(L, (n), len);
    if (!s)
        table_type_error(L, n, nname, LUA_TSTRING);
    return s;
}

static int getfield(lua_State* L, const char* key, int d)
{
    int res;
    lua_rawgetfield(L, -1, key);
    if (lua_isnumber(L, -1))
        res = (int)lua_tointeger(L, -1);
    else
    {
        if (d < 0)
            luaL_error(L, "field '%s' missing in table", key);
        res = d;
    }
    lua_pop(L, 1);
    return res;
}

static void setfield(lua_State* L, const char* key, int value)
{
    lua_pushinteger(L, value);
    lua_setfield(L, -2, key);
}

static long getfield(lua_State* L, const char* key, long d)
{
    long res;
    lua_rawgetfield(L, -1, key);
    if (lua_isnumber(L, -1))
        res = (long)lua_tolong(L, -1);
    else
    {
        if (d < 0)
            luaL_error(L, "field '%s' missing in table", key);
        res = d;
    }
    lua_pop(L, 1);
    return res;
}

static void setfield(lua_State* L, const char* key, long value)
{
    lua_pushlong(L, value);
    lua_setfield(L, -2, key);
}

static long long getfield(lua_State* L, const char* key, long long d)
{
    long long res;
    lua_rawgetfield(L, -1, key);
    if (lua_isnumber(L, -1))
        res = (long long)lua_tollong(L, -1);
    else
    {
        if (d < 0)
            luaL_error(L, "field '%s' missing in table", key);
        res = d;
    }
    lua_pop(L, 1);
    return res;
}

static void setfield(lua_State* L, const char* key, long long value)
{
    lua_pushllong(L, value);
    lua_setfield(L, -2, key);
}

static double getfield(lua_State* L, const char* key, double d)
{
    double res;
    lua_rawgetfield(L, -1, key);
    if (lua_isnumber(L, -1))
        res = (double)lua_tonumber(L, -1);
    else
    {
        if (d < 0)
            luaL_error(L, "field '%s' missing in table", key);
        res = d;
    }
    lua_pop(L, 1);
    return res;
}

static void setfield(lua_State* L, const char* key, double value)
{
    lua_pushnumber(L, value);
    lua_setfield(L, -2, key);
}

static const char* getfield(lua_State* L, const char* key, const char* d)
{
    const char* res;
    lua_rawgetfield(L, -1, key);
    if (lua_isstring(L, -1))
        res = (const char*)lua_tostring(L, -1);
    else
    {
        if (d < 0)
            luaL_error(L, "field '%s' missing in table", key);
        res = d;
    }
    lua_pop(L, 1);
    return res;
}

static void setfield(lua_State* L, const char* key, const char* value)
{
    lua_pushstring(L, value);
    lua_setfield(L, -2, key);
}

static int getboolfield(lua_State* L, const char* key)
{
    int res;
    lua_rawgetfield(L, -1, key);
    res = lua_isnil(L, -1) ? -1 : lua_toboolean(L, -1);
    lua_pop(L, 1);
    return res;
}

static void setboolfield(lua_State* L, const char* key, int value)
{
    if (value < 0) /* undefined? */
        return;    /* does not set field */
    lua_pushboolean(L, value);
    lua_setfield(L, -2, key);
}

static cpr::Authentication getAuthenticationFromArgs(lua_State* L, int idx)
{
    //luaL_checktype(L, idx, LUA_TTABLE);
    checktablefortable(L, idx, "basic authentication");
    lua_rawgeti(L, idx, 1);
    int idt = idx < 0 ? idx - 1 : idx;
    lua_rawgeti(L, idt, 2);
    const char *username = checktableforstring(L, -2, "username");
    const char *password = checktableforstring(L, -1, "password");
    lua_pop(L, 2);
    return cpr::Authentication(username, password);
}

static cpr::Digest getDigestFromArgs(lua_State* L, int idx)
{
    //luaL_checktype(L, idx, LUA_TTABLE);
    checktablefortable(L, idx, "digest authentication");
    lua_rawgeti(L, idx, 1);
    int idt = idx < 0 ? idx - 1 : idx;
    lua_rawgeti(L, idt, 2);
    const char *username = checktableforstring(L, -2, "username");
    const char *password = checktableforstring(L, -1, "password");
    lua_pop(L, 2);
    return cpr::Digest(username, password);
}

static cpr::NTLM getNTLMFromArgs(lua_State* L, int idx)
{
    //luaL_checktype(L, idx, LUA_TTABLE);
    checktablefortable(L, idx, "NTLM authentication");
    lua_rawgeti(L, idx, 1);
    int idt = idx < 0 ? idx - 1 : idx;
    lua_rawgeti(L, idt, 2);
    const char *username = checktableforstring(L, -2, "username");
    const char *password = checktableforstring(L, -1, "password");
    lua_pop(L, 2);
    return cpr::NTLM(username, password);
}

static cpr::Bearer getBearerFromArgs(lua_State* L, int idx)
{
    const char* accessToken = checktableforstring(L, idx, "bearer access token");
    return cpr::Bearer(accessToken);
}

static cpr::Parameters getParametersFromArgs(lua_State* L, int idx)
{
    //luaL_checktype(L, idx, LUA_TTABLE);
    checktablefortable(L, idx, "parameters");
    cpr::Parameters parameters = {};

    // push nil for lua_next to indicate it needs to pick the first key 
    lua_pushnil(L);
    
    int idt = idx < 0 ? idx - 1 : idx;
    while (lua_next(L, idt)) {

        // i.e. when the key is not a number, ignore it;
        if (lua_type(L, -2) != LUA_TNUMBER) {
            lua_pop(L, 1); /* pop the value */ 
            continue;
        }
        
        int k = (int)lua_tointeger(L, -2);

        // key is at -2 on the stack, value at -1. We need to pop the value, 
        // but leave the key on the stack so that lua_next knows where to 
        // continue. You can do anything to process them at this point. 

        int para = -1;
        checktablefortable(L, para, "parameter");
        lua_rawgeti(L, para--, 1);
        lua_rawgeti(L, para, 2);
        std::string key = checktableforstring(L, -2, "key");
        std::string value = checktableforstring(L, -1, "value");
        lua_pop(L, 2);
        parameters.Add(cpr::Parameter(key, value));

        // pop the value when you're done with it 
        lua_pop(L, 2);
        lua_pushinteger(L, k);
    }
    return parameters;
}

static cpr::Header getHeaderFromArgs(lua_State* L, int idx)
{
    //luaL_checktype(L, idx, LUA_TTABLE);
    checktablefortable(L, idx, "header");
    cpr::Header header = {};

    // push nil for lua_next to indicate it needs to pick the first key 
    lua_pushnil(L);
                
    int idt = idx < 0 ? idx - 1 : idx;
    while (lua_next(L, idt)) {

        // i.e. when the key is not a number, ignore it;
        if (lua_type(L, -2) != LUA_TNUMBER) {
            lua_pop(L, 1); /* pop the value */ 
            continue;
        }

        int k = (int)lua_tointeger(L, -2);

        // key is at -2 on the stack, value at -1. We need to pop the value, 
        // but leave the key on the stack so that lua_next knows where to 
        // continue. You can do anything to process them at this point. 

        int h = -1;
        checktablefortable(L, h, "header");
        lua_rawgeti(L, h--, 1);
        lua_rawgeti(L, h, 2);
        std::string key = checktableforstring(L, -2, "key");
        std::string value = checktableforstring(L, -1, "value");
        lua_pop(L, 2);
        header.insert(std::pair<std::string,std::string>(key, value));

        // pop the value when you're done with it 
        lua_pop(L, 2);
        lua_pushinteger(L, k);
    }
    return header;
}

static cpr::Payload getPayloadFromArgs(lua_State* L, int idx)
{
    //luaL_checktype(L, idx, LUA_TTABLE);
    checktablefortable(L, idx, "payload");
    cpr::Payload payload = {};

    // push nil for lua_next to indicate it needs to pick the first key 
    lua_pushnil(L);
                
    int idt = idx < 0 ? idx - 1 : idx;
    while (lua_next(L, idt)) {

        // i.e. when the key is not a number, ignore it;
        if (lua_type(L, -2) != LUA_TNUMBER) {
            lua_pop(L, 1); /* pop the value */ 
            continue;
        }

        int k = (int)lua_tointeger(L, -2);

        // key is at -2 on the stack, value at -1. We need to pop the value, 
        // but leave the key on the stack so that lua_next knows where to 
        // continue. You can do anything to process them at this point. 

        int h = -1;
        checktablefortable(L, h, "header");
        lua_rawgeti(L, h--, 1);
        lua_rawgeti(L, h, 2);
        std::string key = checktableforstring(L, -2, "key");
        std::string value = checktableforstring(L, -1, "value");
        lua_pop(L, 2);
        payload.Add(cpr::Pair(key, value));

        // pop the value when you're done with it 
        lua_pop(L, 2);
        lua_pushinteger(L, k);
    }
    return payload;
}

static cpr::Multipart getMultipartFromArgs(lua_State* L, int idx)
{
    //luaL_checktype(L, idx, LUA_TTABLE);
    checktablefortable(L, idx, "multipart");
    cpr::Multipart multipart = {};

    // push nil for lua_next to indicate it needs to pick the first key 
    lua_pushnil(L);
                
    int idt = idx < 0 ? idx - 1 : idx;
    while (lua_next(L, idt)) {

        // i.e. when the key is not a number, ignore it;
        if (lua_type(L, -2) != LUA_TNUMBER) {
            lua_pop(L, 1); /* pop the value */ 
            continue;
        }

        int k = (int)lua_tointeger(L, -2);

        // key is at -2 on the stack, value at -1. We need to pop the value, 
        // but leave the key on the stack so that lua_next knows where to 
        // continue. You can do anything to process them at this point. 

        int h = -1;
        checktablefortable(L, h, "header");
        lua_rawgeti(L, h--, 1);
        lua_rawgeti(L, h--, 2);
        lua_rawgeti(L, h, 3);
        std::string key = checktableforstring(L, -3, "key");
        std::string value = checktableforstring(L, -2, "value");
        cpr::Part part = cpr::Part(key, value);
        if (!lua_isnoneornil(L, -1))
        {
            std::string content_type = checktableforstring(L, -1, "content type");
            part = cpr::Part(key, value, content_type);
        }
        multipart.parts.push_back(part);
        lua_pop(L, 3);

        // pop the value when you're done with it 
        lua_pop(L, 2);
        lua_pushinteger(L, k);
    }
    return multipart;
}

static cpr::Url getURLFromArgs(lua_State* L, int idx)
{
    size_t linkl;
    //const char* link = luaL_checklstring(L, idx, &linkl);
    const char* link = checktableforlstring(L, idx, "URL", &linkl);
    return cpr::Url(link, linkl);
}

static cpr::Body getBodyFromArgs(lua_State* L, int idx)
{
    size_t bodyl;
    //const char* body = luaL_checklstring(L, idx, &bodyl);
    const char* body = checktableforlstring(L, idx, "body", &bodyl);
    return cpr::Body(body, bodyl);
}

static std::tuple<cpr::Timeout, cpr::Redirect, Auth, cpr::Payload, cpr::Multipart, cpr::Parameters> getOptionsFromArgs(lua_State* L, int idx)
{
    luaL_checktype(L, idx, LUA_TTABLE);
    cpr::Timeout timeout = cpr::Timeout(0); // Default is no timeout
    cpr::Redirect redirect = cpr::Redirect(true); // Default is true
    Auth auth = Auth();
    cpr::Payload payload = {};
    cpr::Parameters parameters = {};
    cpr::Multipart multipart = {};
    
    // push nil for lua_next to indicate it needs to pick the first key 
    lua_pushnil(L);

    while (lua_next(L, idx)) {
        // key is at -2 on the stack, value at -1. We need to pop the value, 
        // but leave the key on the stack so that lua_next knows where to 
        // continue. You can do anything to process them at this point. 

        // i.e. when the key is not a string, ignore it;
        if (lua_type(L, -2) != LUA_TSTRING) {
            lua_pop(L, 1); /* pop the value */ 
            continue;
        }

        const char* k = lua_tostring(L, -2);
        std::string key = k;

        if (key == "followRedirects" || key == "followredirects" || key == "Followredirects" || key == "FollowRedirects" || key == "FOLLOWREDIRECTS")
        {
            if (lua_isboolean(L, -1))
                redirect.follow = lua_toboolean(L, -1) ? true : false;
        }
        else if (key == "maxRedirects" || key == "maxredirects" || key == "Maxredirects" || key == "MaxRedirects" || key == "MAXREDIRECTS")
        {
            int isnum;
            int n = lua_tointegerx(L, -1, &isnum);
            if (isnum){
                if (n < 0)
                    table_arg_error(L, "maximum redirects", "out of range");
                redirect.maximum = n;
            }
        }
        else if (key == "timeout" || key == "Timeout" || key == "TimeOut" || key == "timeOut" || key == "TIMEOUT")
        {
            int isnum;
            int n = lua_tointegerx(L, -1, &isnum);
            if (isnum){
                if (n < 0)
                    table_arg_error(L, "timeout", "out of range");
                timeout = cpr::Timeout(n);
            }
        }
        else if (key == "auth" || key == "authentication" || key == "Auth" || key == "Authentication" || key == "AUTH" || key == "AUTHENTICATION" || key == "basic-auth" || key == "basicAuth" || key == "basicAuthentication" || key == "basicauth" || key == "basicauthentication" || key == "Basicauth" || key == "Basicauthentication" || key == "BasicAuth" || key == "BasicAuthentication" || key == "BASICAUTH" || key == "BASICAUTHENTICATION")
            auth.AddBasic(getAuthenticationFromArgs(L, -1));
        else if (key == "digest" || key == "Digest")
            auth.AddDigest(getDigestFromArgs(L, -1));
        else if (key == "ntlm" || key == "NTLM" || key == "Ntlm")
            auth.AddNTLM(getNTLMFromArgs(L, -1));
        else if (key == "bearer" || key == "BEARER" || key == "Bearer")
            auth.AddBearer(getBearerFromArgs(L, -1));
        else if (key == "parameters" || key == "PARAMETERS" || key == "Parameters")
            parameters = getParametersFromArgs(L, -1);
        else if (key == "payload" || key == "PAYLOAD" || key == "Payload")
            payload = getPayloadFromArgs(L, -1);
        else if (key == "multipart" || key == "MULTIPART" || key == "Multipart" || key == "MultiPart" || key == "multiPart")
            multipart = getMultipartFromArgs(L, -1);

        // pop the value when you're done with it 
        lua_pop(L, 2);
        lua_pushstring(L, k);
    }

    return std::tuple <cpr::Timeout, cpr::Redirect, Auth, cpr::Payload, cpr::Multipart, cpr::Parameters>(timeout, redirect, auth, payload, multipart, parameters);
}

static const char * getTextForEnum( cpr::ErrorCode code )
{
  switch( code )
  {
    case cpr::ErrorCode::OK:
        return "OK";
    case cpr::ErrorCode::CONNECTION_FAILURE:
        return "CONNECTION_FAILURE";
    case cpr::ErrorCode::EMPTY_RESPONSE:
        return "EMPTY_RESPONSE";
    case cpr::ErrorCode::HOST_RESOLUTION_FAILURE:
        return "HOST_RESOLUTION_FAILURE";
    case cpr::ErrorCode::INTERNAL_ERROR:
        return "INTERNAL_ERROR";
    case cpr::ErrorCode::INVALID_URL_FORMAT:
        return "INVALID_URL_FORMAT";
    case cpr::ErrorCode::NETWORK_RECEIVE_ERROR:
        return "NETWORK_RECEIVE_ERROR";
    case cpr::ErrorCode::NETWORK_SEND_FAILURE:
        return "NETWORK_SEND_FAILURE";
    case cpr::ErrorCode::OPERATION_TIMEDOUT:
        return "OPERATION_TIMEDOUT";
    case cpr::ErrorCode::PROXY_RESOLUTION_FAILURE:
        return "PROXY_RESOLUTION_FAILURE";
    case cpr::ErrorCode::SSL_CONNECT_ERROR:
        return "SSL_CONNECT_ERROR";
    case cpr::ErrorCode::SSL_LOCAL_CERTIFICATE_ERROR:
        return "SSL_LOCAL_CERTIFICATE_ERROR";
    case cpr::ErrorCode::SSL_REMOTE_CERTIFICATE_ERROR:
        return "SSL_REMOTE_CERTIFICATE_ERROR";
    case cpr::ErrorCode::SSL_CACERT_ERROR:
        return "SSL_CACERT_ERROR";
    case cpr::ErrorCode::GENERIC_SSL_ERROR:
        return "GENERIC_SSL_ERROR";
    case cpr::ErrorCode::UNSUPPORTED_PROTOCOL:
        return "UNSUPPORTED_PROTOCOL";
    case cpr::ErrorCode::REQUEST_CANCELLED:
        return "REQUEST_CANCELLED";
    case cpr::ErrorCode::TOO_MANY_REDIRECTS:
        return "TOO_MANY_REDIRECTS";
    case cpr::ErrorCode::UNKNOWN_ERROR:
        return "UNKNOWN_ERROR";
    default:
        return "UNKNOWN";
  }
}

static Table *pushResponse (lua_State *L, cpr::Response* response)
{
    lua_createtable(L, 0, 12); /* 12 = number of fields */

    setfield(L, "reason", response->reason.c_str());
    setfield(L, "redirects", response->redirect_count);
    
    lua_createtable(L, 0, 2); /* 2 = number of fields */
    Table* st = hvalue(L->top - 1);
    setfield(L, "code", response->status_code);
    setfield(L, "line", response->status_line.c_str());
    st->readonly = true;
    lua_setfield(L, -2, "status");

    setfield(L, "text", response->text.c_str());
    setfield(L, "url", response->url.data());
    setfield(L, "rawHeader", response->raw_header.c_str());

    cpr::Header header = response->header;
    size_t hsizet = header.size();
    if ( hsizet > INT_MAX )
    {
        luaL_error(L, "header size is larger than INT_MAX");
        //throw std::overflow_error("data is larger than INT_MAX");
    }
    int hsize = static_cast<int>(hsizet);

    lua_createtable(L, 0, hsize);
    Table* ht = hvalue(L->top - 1);

    // Create a map iterator and point to beginning of map
    cpr::Header::iterator hit = header.begin();
    // Iterate over the map using Iterator till end.
    while (hit != header.end())
    {
        // Accessing KEY from element pointed by it.
        std::string k = hit->first;
        // Accessing VALUE from element pointed by it.
        std::string v = hit->second;
        // Add to cookie table
        setfield(L, k.c_str(), v.c_str());
        // Increment the Iterator to point to next entry
        hit++;
    }
    ht->readonly = true;
    lua_setfield(L, -2, "header");

    lua_createtable(L, 0, 2); /* 2 = number of fields */
    Table* et = hvalue(L->top - 1);
    setfield(L, "code", getTextForEnum(response->error.code));
    setfield(L, "message", response->error.message.c_str());
    et->readonly = true;
    lua_setfield(L, -2, "error");

    setfield(L, "downloaded", response->downloaded_bytes);
    setfield(L, "uploaded", response->uploaded_bytes);
    setfield(L, "elapsed", response->elapsed);

    cpr::Cookies cookies = response->cookies;
    int csize = 0;

    // Create a map iterator and point to beginning of map
    cpr::Cookies::iterator sit = cookies.begin();
    // Iterate over the map using Iterator till end.
    while (sit != cookies.end())
    {
        // Increment the Iterator to point to next entry
        csize++;
        sit++;
    }

    lua_createtable(L, 0, csize);
    Table* ct = hvalue(L->top - 1);

    // Create a map iterator and point to beginning of map
    cpr::Cookies::iterator it = cookies.begin();
    // Iterate over the map using Iterator till end.
    while (it != cookies.end())
    {
        // Accessing KEY from element pointed by it.
        std::string k = it->first;
        // Accessing VALUE from element pointed by it.
        std::string v = it->second;
        // Add to cookie table
        setfield(L, k.c_str(), v.c_str());
        // Increment the Iterator to point to next entry
        it++;
    }
    ct->readonly = true;
    lua_setfield(L, -2, "cookies");

    std::vector<std::string> certInfo = response->GetCertInfo();
    size_t data = certInfo.size();
    if ( data > INT_MAX )
    {
        luaL_error(L, "data is larger than INT_MAX");
        //throw std::overflow_error("data is larger than INT_MAX");
    }
    int convertData = static_cast<int>(data);
    lua_createtable(L, convertData, 0); /* 2 = number of fields */
    Table* t = hvalue(L->top - 1);
    int i = 0;
    for (const std::string& s : certInfo)
    {
        TValue* e = &t->array[i++];
        lua_pushstring(L, s.c_str());
        StkId v = L->top - 1;
        setobj2t(L, e, v);
        lua_pop(L, 1);
    }
    t->readonly = true;
    lua_setfield(L, -2, "certInfo");
    luaL_setmetatable(L, CPR_RESPONSE);

    Table* rt = hvalue(L->top - 1);
    rt->readonly = true;
    return rt;
}

static int cprrequest(lua_State* L, RequestMethod method)
{
    cpr::Url url = getURLFromArgs(L, 1);

    if (!lua_isnoneornil(L, 2))
    {
        cpr::Header header = getHeaderFromArgs(L, 2);
        if (!lua_isnoneornil(L, 3))
        {
            cpr::Body body = getBodyFromArgs(L, 3);
            if (!lua_isnoneornil(L, 4)) // URL, Headers, Body, and CustomOptions are available
            {
                luaL_checktype(L, 4, LUA_TTABLE);
                std::tuple<cpr::Timeout, cpr::Redirect, Auth, cpr::Payload, cpr::Multipart, cpr::Parameters> options = getOptionsFromArgs(L, 4);
                cpr::Timeout timeout = std::get<cpr::Timeout>(options); // Default is no timeout
                cpr::Redirect redirect = std::get<cpr::Redirect>(options); // Default is true
                Auth auth = std::get<Auth>(options);
                cpr::Payload payload = std::get<cpr::Payload>(options);
                cpr::Multipart multipart = std::get<cpr::Multipart>(options);
                cpr::Parameters parameters = std::get<cpr::Parameters>(options);
                if (auth.hasBasic)
                {
                    cpr::Response r = Request(method, url, header, body, timeout, redirect, payload, multipart, parameters, auth.basic);
                    pushResponse(L, &r);
                }
                else if (auth.hasDigest)
                {
                    cpr::Response r = Request(method, url, header, body, timeout, redirect, payload, multipart, parameters, auth.digest);
                    pushResponse(L, &r);
                }
                else if (auth.hasNTLM)
                {
                    cpr::Response r = Request(method, url, header, body, timeout, redirect, payload, multipart, parameters, auth.ntlm);
                    pushResponse(L, &r);
                }
                else if (auth.hasBearer)
                {
                    cpr::Response r = Request(method, url, header, body, timeout, redirect, payload, multipart, parameters, auth.bearer);
                    pushResponse(L, &r);
                }
                else
                {
                    cpr::Response r = Request(method, url, header, body, timeout, redirect, payload, multipart, parameters);
                    pushResponse(L, &r);
                }
            }
            else                        // Only URL, Headers, and Body are available
            {
                cpr::Response r = Request(method, url, header, body);
                pushResponse(L, &r);
            }
        }
        else if (!lua_isnoneornil(L, 4)) // URL, Headers, and CustomOptions are available
        {
            luaL_checktype(L, 4, LUA_TTABLE);
            std::tuple<cpr::Timeout, cpr::Redirect, Auth, cpr::Payload, cpr::Multipart, cpr::Parameters> options = getOptionsFromArgs(L, 4);
            cpr::Timeout timeout = std::get<cpr::Timeout>(options); // Default is no timeout
            cpr::Redirect redirect = std::get<cpr::Redirect>(options); // Default is true
            Auth auth = std::get<Auth>(options);
            cpr::Payload payload = std::get<cpr::Payload>(options);
            cpr::Multipart multipart = std::get<cpr::Multipart>(options);
            cpr::Parameters parameters = std::get<cpr::Parameters>(options);
            if (auth.hasBasic)
            {
                cpr::Response r = Request(method, url, header, timeout, redirect, payload, multipart, parameters, auth.basic);
                pushResponse(L, &r);
            }
            else if (auth.hasDigest)
            {
                cpr::Response r = Request(method, url, header, timeout, redirect, payload, multipart, parameters, auth.digest);
                pushResponse(L, &r);
            }
            else if (auth.hasNTLM)
            {
                cpr::Response r = Request(method, url, header, timeout, redirect, payload, multipart, parameters, auth.ntlm);
                pushResponse(L, &r);
            }
            else if (auth.hasBearer)
            {
                cpr::Response r = Request(method, url, header, timeout, redirect, payload, multipart, parameters, auth.bearer);
                pushResponse(L, &r);
            }
            else
            {
                cpr::Response r = Request(method, url, header, timeout, redirect, payload, multipart, parameters);
                pushResponse(L, &r);
            }
        }
        else                        // Only URL and Headers are available
        {
            cpr::Response r = Request(method, url, header);
            pushResponse(L, &r);
        }
    }
    else if (!lua_isnoneornil(L, 3))
    {
        cpr::Body body = getBodyFromArgs(L, 3);
        if (!lua_isnoneornil(L, 4)) // URL, Body, and CustomOptions are available
        {
            luaL_checktype(L, 4, LUA_TTABLE);
            std::tuple<cpr::Timeout, cpr::Redirect, Auth, cpr::Payload, cpr::Multipart, cpr::Parameters> options = getOptionsFromArgs(L, 4);
            cpr::Timeout timeout = std::get<cpr::Timeout>(options); // Default is no timeout
            cpr::Redirect redirect = std::get<cpr::Redirect>(options); // Default is true
            Auth auth = std::get<Auth>(options);
            cpr::Payload payload = std::get<cpr::Payload>(options);
            cpr::Multipart multipart = std::get<cpr::Multipart>(options);
            cpr::Parameters parameters = std::get<cpr::Parameters>(options);
            if (auth.hasBasic)
            {
                cpr::Response r = Request(method, url, body, timeout, redirect, payload, multipart, parameters, auth.basic);
                pushResponse(L, &r);
            }
            else if (auth.hasDigest)
            {
                cpr::Response r = Request(method, url, body, timeout, redirect, payload, multipart, parameters, auth.digest);
                pushResponse(L, &r);
            }
            else if (auth.hasNTLM)
            {
                cpr::Response r = Request(method, url, body, timeout, redirect, payload, multipart, parameters, auth.ntlm);
                pushResponse(L, &r);
            }
            else if (auth.hasBearer)
            {
                cpr::Response r = Request(method, url, body, timeout, redirect, payload, multipart, parameters, auth.bearer);
                pushResponse(L, &r);
            }
            else
            {
                cpr::Response r = Request(method, url, body, timeout, redirect, payload, multipart, parameters);
                pushResponse(L, &r);
            }
        }
        else                        // Only URL and Body are available
        {
            cpr::Response r = Request(method, url, body);
            pushResponse(L, &r);
        }
    }
    else if (!lua_isnoneornil(L, 4)) // URL and CustomOptions are available
    {
        luaL_checktype(L, 4, LUA_TTABLE);
        std::tuple<cpr::Timeout, cpr::Redirect, Auth, cpr::Payload, cpr::Multipart, cpr::Parameters> options = getOptionsFromArgs(L, 4);
        cpr::Timeout timeout = std::get<cpr::Timeout>(options); // Default is no timeout
        cpr::Redirect redirect = std::get<cpr::Redirect>(options); // Default is true
        Auth auth = std::get<Auth>(options);
        cpr::Payload payload = std::get<cpr::Payload>(options);
        cpr::Multipart multipart = std::get<cpr::Multipart>(options);
        cpr::Parameters parameters = std::get<cpr::Parameters>(options);
        if (auth.hasBasic)
        {
            cpr::Response r = Request(method, url, timeout, redirect, payload, multipart, parameters, auth.basic);
            pushResponse(L, &r);
        }
        else if (auth.hasDigest)
        {
            cpr::Response r = Request(method, url, timeout, redirect, payload, multipart, parameters, auth.digest);
            pushResponse(L, &r);
        }
        else if (auth.hasNTLM)
        {
            cpr::Response r = Request(method, url, timeout, redirect, payload, multipart, parameters, auth.ntlm);
            pushResponse(L, &r);
        }
        else if (auth.hasBearer)
        {
            cpr::Response r = Request(method, url, timeout, redirect, payload, multipart, parameters, auth.bearer);
            pushResponse(L, &r);
        }
        else
        {
            cpr::Response r = Request(method, url, timeout, redirect, payload, multipart, parameters);
            pushResponse(L, &r);
        }
    }
    else                             // Only url is available
    {
        cpr::Response r = Request(method, url);
        pushResponse(L, &r);
    }

    return 1;
}

static int cprrequest(lua_State* L)
{
    std::string m = luaL_checklstring(L, (1), NULL);
    lua_remove(L, 1);
    if (requestMap[m] == RequestMethod::None){
        luaL_error(L, "invalid request method (got \"%s\")", m.c_str());
        return 0;
    }
    return cprrequest(L, requestMap[m]);
}

static int cprget(lua_State* L)
{
    return cprrequest(L, RequestMethod::Get);
}

static int cprpost(lua_State* L)
{
    return cprrequest(L, RequestMethod::Post);
}

static int cprdelete(lua_State* L)
{
    return cprrequest(L, RequestMethod::Delete);
}

static int cprpatch(lua_State* L)
{
    return cprrequest(L, RequestMethod::Patch);
}

static int cprput(lua_State* L)
{
    return cprrequest(L, RequestMethod::Put);
}

static int cproptions(lua_State* L)
{
    return cprrequest(L, RequestMethod::Options);
}

static int cprhead(lua_State* L)
{
    return cprrequest(L, RequestMethod::Head);
}

static int response_tostring (lua_State *L)
{
  lua_pushstring(L, CPR_RESPONSE);
  return 1;
}

static const luaL_Reg response_meta[] = {
  {"__tostring", response_tostring},
  {NULL, NULL},
};


static void createmeta (lua_State *L) {
  luaL_newmetatable(L, CPR_RESPONSE);  /* metatable for cpr responses */
  luaL_register(L, NULL, response_meta);    /* fill metatable */
  lua_pushliteral(L, "__index");
  lua_pushvalue(L, -3);               /* dup methods table*/
  lua_rawset(L, -3);                  /* metatable.__index = methods */
  lua_pushliteral(L, "__metatable");
  lua_pushvalue(L, -3);               /* dup methods table*/
  lua_rawset(L, -3);                  /* hide metatable: metatable.__metatable = methods */
  lua_pop(L, 1);                      /* drop metatable */
  lua_pop(L, 1);  /* pop metatable */
}

static const luaL_Reg funcs[] = {
  {"request", cprrequest},
  {"get", cprget},
  {"post", cprpost},
  {"patch", cprpatch},
  {"put", cprput},
  {"delete", cprdelete},
  {"options", cproptions},
  {"head", cprhead},
  {NULL, NULL},
};

LUALIB_API int luaopen_cpr(lua_State* L)
{
  luaL_register(L, LUA_CPRLIBNAME, funcs);  /* new module */

//   lua_newtable(L);                          // create the response table
//   luaL_register(L, NULL, response_methods);  // register functions into response table
//   lua_setfield(L, -2, "response");          // add response table to module
//   lua_pop(L, 1);                      /* drop methods */
//   lua_pop(L, 1);                      /* drop response */

  createmeta(L);

  return 1;
}
