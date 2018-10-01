-- Copyright (C) 2016-2017 Jian Chang <aa65535@live.com>
-- Licensed to the public under the GNU General Public License v3.

local m, s, o
local shadowsocksr = "shadowsocksr"

local function has_bin(name)
	return luci.sys.call("command -v %s >/dev/null" %{name}) == 0
end

m = Map(shadowsocksr, "%s - %s" %{translate("ShadowSocksR"), translate("Servers Manage")})

-- Server Subscribe
if nixio.fs.access("/usr/share/shadowsocksr/subscribe.sh") and has_bin("base64") and has_bin("curl") and has_bin("bash") and has_bin("dig") then
	s = m:section(TypedSection, "server_subscribe", translate("Server subscription"))
	s.anonymous = true

	o = s:option(Flag, "auto_update", translate("Auto Update"))
	o.rmempty = false

	o = s:option(Flag, "proxy", translate("Through proxy update"))
	o.rmempty = false

	o = s:option(ListValue, "auto_update_time", translate("Update time (every day)"))
	for t = 0,23 do
	o:value(t, t..":00")
	end
	o.default=2
	o.rmempty = false

	o = s:option(DynamicList, "subscribe_url", translate("Subscribe URL"))
	o.rmempty = true

	o = s:option(Button,"update",translate("Update"))
	o.write = function()
	luci.sys.call("/usr/share/shadowsocksr/subscribe.sh >/dev/null 2>&1")
	luci.http.redirect(luci.dispatcher.build_url("admin", "services", "shadowsocksr", "servers"))
	end
end

-- [[ Servers Manage ]]--
s = m:section(TypedSection, "servers")
s.anonymous = true
s.addremove = true
s.sortable = true
s.template = "cbi/tblsection"
s.extedit = luci.dispatcher.build_url("admin/services/shadowsocksr/servers/%s")
function s.create(...)
	local sid = TypedSection.create(...)
	if sid then
		luci.http.redirect(s.extedit % sid)
		return
	end
end

o = s:option(DummyValue, "alias", translate("Alias"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end

o = s:option(DummyValue, "server", translate("Server Address"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or "?"
end

o = s:option(DummyValue, "server_port", translate("Server Port"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or "?"
end

o = s:option(DummyValue, "encrypt_method", translate("Encrypt Method"))
function o.cfgvalue(...)
	local v = Value.cfgvalue(...)
	return v and v:upper() or "?"
end

o = s:option(DummyValue, "protocol", translate("Protocol"))
function o.cfgvalue(...)
	local v = Value.cfgvalue(...)
	return v and v:upper() or "?"
end

o = s:option(DummyValue, "obfs", translate("Obfs"))
function o.cfgvalue(...)
	local v = Value.cfgvalue(...)
	return v and v:upper() or "?"
end

return m
