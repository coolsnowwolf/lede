--[[

LuCI Squid module

Copyright (C) 2015, OpenWrt.org

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

Author: Marko Ratkaj <marko.ratkaj@sartura.hr>

]]--

local fs = require "nixio.fs"
local sys = require "luci.sys"
require "ubus"

m = Map("squid", translate("Squid"))
m.on_after_commit = function() luci.sys.call("/etc/init.d/squid restart") end

s = m:section(TypedSection, "squid")
s.anonymous = true
s.addremove = false

s:tab("general", translate("General Settings"))

http_port = s:taboption("general", Value, "http_port", translate("Port"))
http_port.datatype = "portrange"
http_port.placeholder = "0-65535"

visible_hostname = s:taboption("general", Value, "visible_hostname", translate("Visible Hostname"))
visible_hostname.datatype="string"
visible_hostname.placeholder = "OpenWrt"

coredump_dir = s:taboption("general", Value, "coredump_dir", translate("Coredump files directory"))
coredump_dir.datatype="string"
coredump_dir.placeholder = "/tmp/squid"

s:tab("advanced", translate("Advanced Settings"))

squid_config_file = s:taboption("advanced", TextValue, "_data", "")
squid_config_file.wrap = "off"
squid_config_file.rows = 25
squid_config_file.rmempty = false

function squid_config_file.cfgvalue()
	local uci = require "luci.model.uci".cursor_state()
	local file = uci:get("squid", "squid", "config_file")
	if file then
		return fs.readfile(file) or ""
	else
		return ""
	end
end

function squid_config_file.write(self, section, value)
    if value then
		local uci = require "luci.model.uci".cursor_state()
		local file = uci:get("squid", "squid", "config_file")
	fs.writefile(file, value:gsub("\r\n", "\n"))
    end
end

return m
