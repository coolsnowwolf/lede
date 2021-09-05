-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008-2011 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local ipkgfile = "/etc/opkg.conf"
local distfeeds = "/etc/opkg/distfeeds.conf"
local customfeeds = "/etc/opkg/customfeeds.conf"

f = SimpleForm("ipkgconf", translate("OPKG-Configuration"), translate("General options for opkg"))

f:append(Template("admin_system/ipkg"))

t = f:field(TextValue, "lines")
t.wrap = "off"
t.rows = 10
function t.cfgvalue()
	return nixio.fs.readfile(ipkgfile) or ""
end

function t.write(self, section, data)
	return nixio.fs.writefile(ipkgfile, data:gsub("\r\n", "\n"))
end

function f.handle(self, state, data)
	return true
end

g = SimpleForm("distfeedconf", translate("Distribution feeds"),
	translate("Build/distribution specific feed definitions. This file will NOT be preserved in any sysupgrade."))

d = g:field(TextValue, "lines2")
d.wrap = "off"
d.rows = 10
function d.cfgvalue()
	return nixio.fs.readfile(distfeeds) or ""
end

function d.write(self, section, data)
	return nixio.fs.writefile(distfeeds, data:gsub("\r\n", "\n"))
end

function g.handle(self, state, data)
	return true
end

h = SimpleForm("customfeedconf", translate("Custom feeds"),
	translate("Custom feed definitions, e.g. private feeds. This file can be preserved in a sysupgrade."))

c = h:field(TextValue, "lines3")
c.wrap = "off"
c.rows = 10
function c.cfgvalue()
	return nixio.fs.readfile(customfeeds) or ""
end

function c.write(self, section, data)
	return nixio.fs.writefile(customfeeds, data:gsub("\r\n", "\n"))
end

function h.handle(self, state, data)
	return true
end

return f, g, h
