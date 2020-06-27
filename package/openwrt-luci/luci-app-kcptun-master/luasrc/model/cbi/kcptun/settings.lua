-- Copyright 2016-2020 Xingwang Liao <kuoruan@gmail.com>
-- Licensed to the public under the Apache License 2.0.

local uci  = require "luci.model.uci".cursor()
local util = require "luci.util"
local sys  = require "luci.sys"
local fs   = require "nixio.fs"

local m, s, o
local server_table = { }

local function get_ip_string(ip)
	if ip and ip:find(":") then
		return "[%s]" % ip
	else
		return ip or ""
	end
end

uci:foreach("kcptun", "servers", function(s)
	if s.alias then
		server_table[s[".name"]] = s.alias
	elseif s.server_addr and s.server_port then
		server_table[s[".name"]] = "%s:%s" % { get_ip_string(s.server_addr), s.server_port }
	end
end)

m = Map("kcptun", "%s - %s" % { translate("Kcptun"), translate("Settings") })
m:append(Template("kcptun/status"))

s = m:section(NamedSection, "general", "general", translate("General Settings"))
s.anonymous = true
s.addremove = false

o = s:option(ListValue, "server", translate("Server"))
o:value("", translate("Disable"))
for k, v in pairs(server_table) do
	o:value(k, v)
end

o = s:option(Value, "client_file", translate("Client File"))
o.rmempty = false

o = s:option(ListValue, "daemon_user", translate("Run Daemon as User"))
o:value("")
for u in util.execi("cat /etc/passwd | cut -d ':' -f1") do
	o:value(u)
end

o = s:option(Flag, "enable_logging", translate("Enable Logging"))
o.rmempty = false

o = s:option(Value, "log_folder", translate("Log Folder"))
o.datatype = "directory"
o.placeholder = "/var/log/kcptun"
o:depends("enable_logging", "1")
o.formvalue = function(...)
	local v = (Value.formvalue(...) or ""):trim()
	if v ~= "" then
		v = string.gsub(v, "\\", "/")
		if v:sub(1, 1) ~= "/" then
			v = "/" .. v
		end

		while v:sub(-1) == "/" do
			v = v:sub(1, -2)
		end
	end

	return v
end
o.validate = function(self, value, section)
	if value and not fs.stat(value) then
		local res, code, msg = fs.mkdir(value)
		if not res then
			return nil, msg
		end
	end
	return Value.validate(self, value, section)
end

o = s:option(Value, "mem_percentage", translate("Memory percentage"),
	translate("The maximum percentage of memory used by Kcptun."))
o.datatype = "range(0, 100)"
o.placeholder = "80"

return m
