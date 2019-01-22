#-- Copyright (C) 2018 dz <dingzhong110@gmail.com>

require("nixio.fs")
require("luci.http")

m = Map("oscam", translate("oscam"))

if luci.sys.call("pidof oscam >/dev/null") == 0 then
	m = Map("oscam", translate("oscam"), "%s - %s" %{translate("oscam"), translate("<strong><font color=\"green\">Running</font></strong>")})
else
	m = Map("oscam", translate("oscam"), "%s - %s" %{translate("oscam"), translate("<strong><font color=\"red\">Not Running</font></strong>")})
end

-- Basic
s = m:section(TypedSection, "oscam", translate("Settings"), translate("General Settings"))
s.anonymous = true

---- Eanble
enable = s:option(Flag, "enabled", translate("Enable"), translate("Enable or disable oscam server"))
enable.default = 0
enable.rmempty = false

-- Doman addresss
s = m:section(TypedSection, "oscam", translate("oscam conf"), 
	translate("oscam conf"))
s.anonymous = true

---- address
addr = s:option(Value, "address",
	translate(""), 
	translate("-------------------------------------------------------------------- " ..
	  "----------------------------------------------------------------------------. " ..
	  ""))

addr.template = "cbi/tvalue"
addr.rows = 30

function addr.cfgvalue(self, section)
	return nixio.fs.readfile("/etc/oscam/oscam.conf")
end

function addr.write(self, section, value)
	value = value:gsub("\r\n?", "\n")
	nixio.fs.writefile("/etc/oscam/oscam.conf", value)
end

return m
