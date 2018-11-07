-- Copyright (C) 2017 yushi studio <ywb94@qq.com> github.com/ywb94
-- Licensed to the public under the GNU General Public License v3.

local m, s, o,kcp_enable
local shadowsocksr = "shadowsocksr"
local uci = luci.model.uci.cursor()
local ipkg = require("luci.model.ipkg")
local fs = require "nixio.fs"
local sys = require "luci.sys"
local sid = arg[1]

local function isKcptun(file)
    if not fs.access(file, "rwx", "rx", "rx") then
        fs.chmod(file, 755)
    end

    local str = sys.exec(file .. " -v | awk '{printf $1}'")
    return (str:lower() == "kcptun")
end


local server_table = {}
local encrypt_methods = {
	"none",
	"table",
	"rc4",
	"rc4-md5-6",
	"rc4-md5",
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
	"verify_deflate",
	"auth_sha1_v4",
	"auth_aes128_sha1",
	"auth_aes128_md5",
	"auth_chain_a",
	"auth_chain_b",
	"auth_chain_c",
	"auth_chain_d",
	"auth_chain_e",
	"auth_chain_f",
}

obfs = {
	"plain",
	"http_simple",
	"http_post",
	"random_head",
	"tls1.2_ticket_auth",
}

m = Map(shadowsocksr, translate("Edit ShadowSocksR Server"))
m.redirect = luci.dispatcher.build_url("admin/services/shadowsocksr/servers")
if m.uci:get(shadowsocksr, sid) ~= "servers" then
	luci.http.redirect(m.redirect) 
	return
end

-- [[ Servers Setting ]]--
s = m:section(NamedSection, sid, "servers")
s.anonymous = true
s.addremove   = false

o = s:option(DummyValue,"ssr_url","SSR URL") 
o.rawhtml  = true
o.template = "shadowsocksr/ssrurl"
o.value =sid

o = s:option(Value, "alias", translate("Alias(optional)"))

-- o = s:option(Flag, "auth_enable", translate("Onetime Authentication"))
-- o.rmempty = false

o = s:option(Value, "server", translate("Server Address"))
o.datatype = "host"
o.rmempty = false

o = s:option(Value, "server_port", translate("Server Port"))
o.datatype = "port"
o.rmempty = false

-- o = s:option(Value, "timeout", translate("Connection Timeout"))
-- o.datatype = "uinteger"
-- o.default = 60
-- o.rmempty = false

o = s:option(Value, "password", translate("Password"))
o.password = true
o.rmempty = false

o = s:option(ListValue, "encrypt_method", translate("Encrypt Method"))
for _, v in ipairs(encrypt_methods) do o:value(v) end
o.rmempty = false

o = s:option(ListValue, "protocol", translate("Protocol"))
for _, v in ipairs(protocol) do o:value(v) end
o.rmempty = false

o = s:option(Value, "protocol_param", translate("Protocol param(optional)"))

o = s:option(ListValue, "obfs", translate("Obfs"))
for _, v in ipairs(obfs) do o:value(v) end
o.rmempty = false

o = s:option(Value, "obfs_param", translate("Obfs param(optional)"))

-- o = s:option(Flag, "fast_open", translate("TCP Fast Open"))
-- o.rmempty = false

o = s:option(Flag, "switch_enable", translate("Enable Auto Switch"))
o.rmempty = false
o.default = "1"

o = s:option(Value, "local_port", translate("Local Port"))
o.datatype = "port"
o.default = 1234
o.rmempty = false

if nixio.fs.access("/usr/bin/ssr-kcptun") then

kcp_enable = s:option(Flag, "kcp_enable", translate("KcpTun Enable"), translate("bin:/usr/bin/ssr-kcptun"))
kcp_enable.rmempty = false

o = s:option(Value, "kcp_port", translate("KcpTun Port"))
o.datatype = "port"
o.default = 4000
function o.validate(self, value, section)
		local kcp_file="/usr/bin/ssr-kcptun"
		local enable = kcp_enable:formvalue(section) or kcp_enable.disabled
		if enable == kcp_enable.enabled then
    if not fs.access(kcp_file)  then
        return nil, translate("Haven't a Kcptun executable file")
    elseif  not isKcptun(kcp_file) then
        return nil, translate("Not a Kcptun executable file")    
    end
    end

    return value
end

o = s:option(Value, "kcp_password", translate("KcpTun Password"))
o.password = true

o = s:option(Value, "kcp_param", translate("KcpTun Param"))
o.default = "--nocomp"

end

return m
