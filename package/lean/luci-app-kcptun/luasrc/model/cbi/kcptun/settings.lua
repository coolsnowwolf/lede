-- Copyright 2016-2017 Xingwang Liao <kuoruan@gmail.com>
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
o.rmempty  = false

o = s:option(ListValue, "daemon_user", translate("Run Daemon as User"))
for u in util.execi("cat /etc/passwd | cut -d ':' -f1") do
	o:value(u)
end

o = s:option(Flag, "enable_logging", translate("Enable Logging"))
o.rmempty  = false

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

o = s:option(ListValue, "arch", translate("CPU Architecture"),
	translate("The ARCH for checking updates." ..
	" Note: Make sure OpenWrt/LEDE 'MIPS FPU Emulator' is enabled on MIPS/MIPSLE devices."))
o:value("", translate("Auto"))
o:value("i386", "x86")
o:value("x86_64", "x86_64")
o:value("armv5", "ARMv5")
o:value("armv6", "ARMv6")
o:value("armv7", "ARMv7+")
o:value("ar71xx", "MIPS")
o:value("ramips", "MIPSLE")

o = s:option(Button, "_check_kcptun", translate("Check Kcptun Update"),
	translate("Make sure that the 'Client File' dictionary has enough space."))
o.template = "kcptun/button"
o.inputstyle = "apply"
o.placeholder = translate("Check Kcptun Update")
o.btnclick = "check_update('kcptun', this);"
o.id = "_kcptun-check_kcptun"

o = s:option(Flag, "save_config", translate("Save Config File"),
	translate("Save config file while upgrade LuCI."))

o = s:option(Button, "_check_luci", translate("Check LuCI Update"),
	translate("You may need to reload current page after update LuCI. Note that translation will not be updated."))
o.template = "kcptun/button"
o.inputstyle = "apply"
o.placeholder = translate("Check LuCI Update")
o.btnclick = "check_update('luci', this);"
o.id = "_kcptun-check_luci"

return m
