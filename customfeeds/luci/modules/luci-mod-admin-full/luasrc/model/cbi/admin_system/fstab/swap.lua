-- Copyright 2010 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local fs   = require "nixio.fs"
local util = require "nixio.util"

local devices = {}
util.consume((fs.glob("/dev/sd*")), devices)
util.consume((fs.glob("/dev/hd*")), devices)
util.consume((fs.glob("/dev/scd*")), devices)
util.consume((fs.glob("/dev/mmc*")), devices)

local size = {}
for i, dev in ipairs(devices) do
	local s = tonumber((fs.readfile("/sys/class/block/%s/size" % dev:sub(6))))
	size[dev] = s and math.floor(s / 2048)
end


m = Map("fstab", translate("Mount Points - Swap Entry"))
m.redirect = luci.dispatcher.build_url("admin/system/fstab")

if not arg[1] or m.uci:get("fstab", arg[1]) ~= "swap" then
	luci.http.redirect(m.redirect)
	return
end


mount = m:section(NamedSection, arg[1], "swap", translate("Swap Entry"))
mount.anonymous = true
mount.addremove = false

mount:tab("general",  translate("General Settings"))
mount:tab("advanced", translate("Advanced Settings"))


mount:taboption("general", Flag, "enabled", translate("Enable this swap")).rmempty = false


o = mount:taboption("general", Value, "device", translate("Device"),
	translate("The device file of the memory or partition (<abbr title=\"for example\">e.g.</abbr> <code>/dev/sda1</code>)"))

for i, d in ipairs(devices) do
	o:value(d, size[d] and "%s (%s MB)" % {d, size[d]})
end

o = mount:taboption("advanced", Value, "uuid", translate("UUID"),
	translate("If specified, mount the device by its UUID instead of a fixed device node"))

o = mount:taboption("advanced", Value, "label", translate("Label"),
	translate("If specified, mount the device by the partition label instead of a fixed device node"))


return m
