-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Cpyright by Twizo<1282055288@qq.com>
-- Licensed to the public under the Apache License 2.0.

local m, s, o
local fs = require "nixio.fs"
local sys = require "luci.sys"

m = Map("mac", translate("Mac address"),
	translate("Clone Wan address")
	..[[<br /><strong>]]
	..[[<a href="https://github.com/Twinzo1/openwrt/tree/master/luci-app-mac" target="_blank">]]
	..translate("Project address")
	..[[</a>]]
	..[[</strong><br />]])

s = m:section(TypedSection, "mac")
-- 这里对应config里面的option
s.addremove = false
s.anonymous = true

os = s:option(Value,"mac",translate(""))
os.template ="mac"

enable=s:option(Flag,"enable",translate("Timing change Mac"))
enable.rmempty = false
enable.default=0

week=s:option(ListValue,"week",translate("Week/Day"))
week:value(0,translate("Everyday"))
week:value(1,translate("Monday"))
week:value(2,translate("Tuesday"))
week:value(3,translate("Wednesday"))
week:value(4,translate("Thursday"))
week:value(5,translate("Friday"))
week:value(6,translate("Saturday"))
week:value(7,translate("Sunday"))
week.default=0
week:depends({enable="1"})

hour=s:option(Value,"hour",translate("Hour"))
hour.datatype = "range(0,23)"
hour.default=0
hour:depends({enable="1"})

pass=s:option(Value,"minute",translate("Minute"))
pass.datatype = "range(0,59)"
pass.default=0
pass:depends({enable="1"})

reboot_run=s:option(Flag,"reboot_run",translate("Modify mac at boot"))
reboot_run.rmempty = true
reboot_run.default=0

local apply = luci.http.formvalue("cbi.apply")
local mac=luci.http.formvalue("c_mac")
if apply then
    luci.sys.exec("uci set mac.@mac[0].macaddr=%s" % mac)
    luci.sys.exec("uci commit mac")
    io.popen("/etc/init.d/mac start")
end


return m
