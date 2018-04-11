-- Copyright 2016-2017 Xingwang Liao <kuoruan@gmail.com>
-- Licensed to the public under the Apache License 2.0.

local dsp = require "luci.dispatcher"
local http = require "luci.http"

local m, s, o

local function get_ip_string(ip)
	if ip and ip:find(":") then
		return "[%s]" % ip
	else
		return ip or ""
	end
end

m = Map("kcptun", "%s - %s" % { translate("Kcptun"), translate("Server List") })

s = m:section(TypedSection, "servers")
s.anonymous = true
s.addremove = true
s.sortable = true
s.template = "cbi/tblsection"
s.extedit = dsp.build_url("admin/services/kcptun/servers/%s")
function s.create(...)
	local sid = TypedSection.create(...)
	if sid then
		m.uci:save("kcptun")
		http.redirect(s.extedit % sid)
		return
	end
end

o = s:option(DummyValue, "alias", translate("Alias"))
function o.cfgvalue(self, section)
	return Value.cfgvalue(self, section) or translate("None")
end

o = s:option(DummyValue, "_server_address", translate("Server Address"))
function o.cfgvalue(self, section)
	local server = m.uci:get("kcptun", section, "server_addr") or "?"
	local server_port = m.uci:get("kcptun", section, "server_port") or "29900"
	return "%s:%s" % { get_ip_string(server), server_port }
end

o = s:option(DummyValue, "_listen_addres", translate("Listen Address"))
function o.cfgvalue(self, section)
	local local_host = m.uci:get("kcptun", section, "listen_addr") or "0.0.0.0"
	local local_port = m.uci:get("kcptun", section, "listen_port") or "12984"
	return "%s:%s" % { get_ip_string(local_host), local_port }
end

o = s:option(DummyValue, "crypt", translate("Encrypt Method"))
function o.cfgvalue(...)
	local v = Value.cfgvalue(...)
	return v and v:upper() or "?"
end

o = s:option(DummyValue, "mode", translate("Embedded Mode"))
function o.cfgvalue(...)
	local v = Value.cfgvalue(...)
	return v and v:upper() or "?"
end

o = s:option(DummyValue, "nocomp", translate("Disable Compression"))
function o.cfgvalue(...)
	local v = Value.cfgvalue(...)
	return v == "true" and translate("True") or translate("False")
end

return m
