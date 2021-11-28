-- This file is part of the Luau programming language and is licensed under MIT License; see LICENSE.txt for details
print("testing cpr lib")

--GET
assert(cpr.get("http://www.httpbin.org/get").status.code == 200)
assert(cpr.get("http://www.httpbin.org/get", nil, nil, {parameters = {{"hello", "world"}}}).url == "http://www.httpbin.org/get?hello=world")

--POST
assert(cpr.post("http://www.httpbin.org/post", {{"Content-Type", "text/plain"}}, "This is raw POST data").status.code == 200)
assert(cpr.post("http://www.httpbin.org/post", nil, nil, {payload = {{"key", "value"}}}).status.code == 200)
assert(cpr.post("http://www.httpbin.org/post", nil, nil, {multipart = {{"key", "value"}}}).status.code == 200)
assert(cpr.post("http://www.httpbin.org/post", nil, nil, {parameters = {{"hello", "world"}, {"stay", "cool"}}}).status.code == 200)

--Authentication
assert(cpr.get("http://www.httpbin.org/basic-auth/user/pass", nil, nil, {auth = {"user","pass"}}).status.code == 200)
assert(cpr.get("http://www.httpbin.org/basic-auth/user/pass").status.code == 401)
assert(cpr.get("http://www.httpbin.org/digest-auth/auth/user/pass", nil, nil, {digest = {"user","pass"}}).status.code == 200)
assert(cpr.get("http://www.httpbin.org/digest-auth/auth/user/pass").status.code == 401)
assert(cpr.get("http://www.httpbin.org/bearer", nil, nil, {bearer = "ACCESS_TOKEN"}).status.code == 200)
assert(cpr.get("http://www.httpbin.org/bearer").status.code == 401)

assert(json.decode(cpr.get("http://www.httpbin.org/get").text).url == "http://www.httpbin.org/get")