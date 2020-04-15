-- Copyright (C) 2017 yushi studio <ywb94@qq.com>
-- Licensed to the public under the GNU General Public License v3.
require "luci.http"
require "luci.dispatcher"
local m, s, o
local shadowsocksr = "shadowsocksr"
local sid = arg[1]

local encrypt_methods = {
"rc4-md5",
"rc4-md5-6",
"rc4",
"table",
"aes-128-cfb",
"aes-192-cfb",
"aes-256-cfb",
"aes-128-ctr",
"aes-192-ctr",
"aes-256-ctr",
"bf-cfb",
"camellia-128-cfb",
"camellia-192-cfb",
"camellia-256-cfb",
"cast5-cfb",
"des-cfb",
"idea-cfb",
"rc2-cfb",
"seed-cfb",
"salsa20",
"chacha20",
"chacha20-ietf",
}

local protocol = {
"origin",
}

obfs = {
"plain",
"http_simple",
"http_post",
}

m = Map(shadowsocksr, translate("Edit ShadowSocksR Server"))

m.redirect = luci.dispatcher.build_url("admin/services/shadowsocksr/server")
if m.uci:get(shadowsocksr, sid) ~= "server_config" then
	luci.http.redirect(m.redirect)
	return
end

-- [[ Server Setting ]]--
s = m:section(NamedSection, sid, "server_config")
s.anonymous = true
s.addremove = false

o = s:option(Flag, "enable", translate("Enable"))
o.default = 1
o.rmempty = false

o = s:option(ListValue, "type", translate("Server Type"))
o:value("ssr", translate("ShadowsocksR"))
o:value("socks5", translate("Socks5"))
o.default = "socks5"

o = s:option(Value, "server_port", translate("Server Port"))
o.datatype = "port"
o.default = 8388
o.rmempty = false

o = s:option(Value, "timeout", translate("Connection Timeout"))
o.datatype = "uinteger"
o.default = 60
o.rmempty = false
o:depends("type", "ssr")

o = s:option(Value, "username", translate("Username"))
o.rmempty = false
o:depends("type", "socks5")

o = s:option(Value, "password", translate("Password"))
o.password = true
o.rmempty = false

o = s:option(ListValue, "encrypt_method", translate("Encrypt Method"))
for _, v in ipairs(encrypt_methods) do o:value(v) end
o.rmempty = false
o:depends("type", "ssr")

o = s:option(ListValue, "protocol", translate("Protocol"))
for _, v in ipairs(protocol) do o:value(v) end
o.rmempty = false
o:depends("type", "ssr")


o = s:option(ListValue, "obfs", translate("Obfs"))
for _, v in ipairs(obfs) do o:value(v) end
o.rmempty = false
o:depends("type", "ssr")

o = s:option(Value, "obfs_param", translate("Obfs param(optional)"))
o:depends("type", "ssr")

o = s:option(Flag, "fast_open", translate("TCP Fast Open"))
o.rmempty = false
o:depends("type", "ssr")

return m
