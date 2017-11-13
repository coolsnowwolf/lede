--[[
LuCI - Lua Configuration Interface

Copyright 2011 flyzjhz <flyzjhz@gmail.com>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

]]--


local fs     = require "nixio.fs"
local uci = require "luci.model.uci".cursor()
local lanipaddr = uci:get("network", "lan", "ipaddr") or "192.168.1.1"
--- Retrieves the output of the "get_oscam_port" command.
-- @return	String containing the current get_oscam_port
function get_oscam_port()

	local oscam_conf= fs.readfile("/usr/oscam/oscam.conf")
	local oscam_conf_port = tonumber(oscam_conf:match("[Hh]ttppor[Tt].-= ([^\n]+)")) or "8899"
	return oscam_conf_port
end

local oscamport = get_oscam_port()

m = Map("oscam", translate("OSCAM","OSCAM"),translate("oscam desc",
"oscam for openwrt"))

s = m:section(TypedSection, "setting", translate("set","settings"))
s.anonymous = true
s.addremove = false

enable = s:option(Flag, "enable", translate("enable", "enable"))
enable.default = false
enable.optional = false
enable.rmempty = false

interval = s:option(Value, "interval", translate("interval","interval"),
translate("interval_desc","interval_desc"))
interval.optional = false
interval.rmempty = false
interval.default = 30

s:option(DummyValue,"oscamweb" ,translate("<a target=\"_blank\" href='http://"..lanipaddr..":"..oscamport.."'>OSCAM Web Intelface</a> "),translate("Open the oscam Web"))

tmpl = s:option(Value, "_tmpl",
	translate("Edit oscam configuration."), 
	translate("This is the content of the file '/usr/oscam/oscam.conf'"))

tmpl.template = "cbi/tvalue"
tmpl.rows = 20

function tmpl.cfgvalue(self, section)
	return nixio.fs.readfile("/usr/oscam/oscam.conf")
end

function tmpl.write(self, section, value)
	value = value:gsub("\r\n?", "\n")
	nixio.fs.writefile("//usr/oscam/oscam.conf", value)
end

return m


