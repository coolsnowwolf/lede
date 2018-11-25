-- Copyright (C) 2017 yushi studio <ywb94@qq.com>
-- Licensed to the public under the GNU General Public License v3.

local m, sec, o
local shadowsocksr = "shadowsocksr"
local uci = luci.model.uci.cursor()
local ipkg = require("luci.model.ipkg")


m = Map(shadowsocksr, translate("ShadowSocksR Server"))

local encrypt_methods = {
	"table",
	"rc4",
	"rc4-md5",
	"rc4-md5-6",
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
}

obfs = {
	"plain",
	"http_simple",
	"http_post",
	"random_head",
	"tls1.2_ticket_auth",
	"tls1.2_ticket_fastauth",
}





-- [[ Global Setting ]]--
sec = m:section(TypedSection, "server_global", translate("Global Setting"))
sec.anonymous = true



o = sec:option(Flag, "enable_server", translate("Enable Server"))
o.rmempty = false

-- [[ Server Setting ]]--
sec = m:section(TypedSection, "server_config", translate("Server Setting"))
sec.anonymous = true
sec.addremove = true
sec.template = "cbi/tblsection"
sec.extedit = luci.dispatcher.build_url("admin/services/shadowsocksr/server/%s")
function sec.create(...)
	local sid = TypedSection.create(...)
	if sid then
		luci.http.redirect(sec.extedit % sid)
		return
	end
end

o = sec:option(Flag, "enable", translate("Enable"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("0")
end
o.rmempty = false

o = sec:option(DummyValue, "server_port", translate("Server Port"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or "?"
end


o = sec:option(DummyValue, "encrypt_method", translate("Encrypt Method"))
function o.cfgvalue(...)
	local v = Value.cfgvalue(...)
	return v and v:upper() or "?"
end

o = sec:option(DummyValue, "protocol", translate("Protocol"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or "?"
end



o = sec:option(DummyValue, "obfs", translate("Obfs"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or "?"
end



return m
