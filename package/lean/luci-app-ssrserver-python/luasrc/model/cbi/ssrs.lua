local a,t,e
local m, s
local o = require"nixio.fs"
local n = {
"none",
"aes-128-ctr",
"aes-192-ctr",
"aes-256-ctr",
"aes-128-cfb",
"aes-192-cfb",
"aes-256-cfb",
"rc4",
"rc4-md5",
"rc4-md5-6",
"chacha20",
"chacha20-ietf-poly1305",
}

local s = {
"origin",
"verify_deflate",
"auth_sha1_v4",
"auth_aes128_md5",
"auth_aes128_sha1",
"auth_chain_a",
"auth_chain_b",
"auth_chain_c",
"auth_chain_d",
"auth_chain_e",
"auth_chain_f",
}

local i={
"plain",
"http_simple",
"http_post",
"random_head",
"tls1.2_ticket_auth",
"tls1.2_ticket_fastauth",
}

local o={
"false",
"true",
}

a = Map("ssrs")
a.title = translate("ShadowSocksR Server Config")
a.description = translate("ShadowsocksR Python Server is a fork of the Shadowsocks project, claimed to be superior in terms of security and stability")

a:section(SimpleSection).template = "ssrs/ssrs_status"

t = a:section(TypedSection,"server",translate(""))
t.anonymous = true
t.addremove = false

e = t:option(Flag, "enable", translate("Enable"))
e.rmempty = false

e = t:option(Value, "server_port", translate("Server Port"))
e.datatype = "port"
e.rmempty = false
e.default = 139

e = t:option(Value, "password", translate("Password"))
e.password = true
e.rmempty = false

e = t:option(ListValue, "encrypt_method", translate("Encrypt Method"))
for a,t in ipairs(n)do e:value(t)end
e.rmempty = false

e = t:option(ListValue, "protocol", translate("Protocol"))
for a,t in ipairs(s)do e:value(t)end
e.rmempty = false

e = t:option(ListValue,"obfs",translate("Obfs"))
for a,t in ipairs(i)do e:value(t)end
e.rmempty = false

return a
