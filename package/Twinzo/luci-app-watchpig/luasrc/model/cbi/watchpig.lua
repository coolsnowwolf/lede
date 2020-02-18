-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Cpyright by Twinzo<1282055288@qq.com>
-- Licensed to the public under the Apache License 2.0.

local m, s, o
local fs = require "nixio.fs"
local sys = require "luci.sys"

m = Map("watchpig", translate("Watchpig"),
	[[<strong>]]
	..translate("Copyright statement: Watchpig is a combination of open source software luci-app-mac and watchcat, it is also open source software")
	..[[</strong><br /><strong>]]
	..translate("To avoid installation conflicts with them, it is named Watchpig")
	..[[</strong><br /><strong>]]
	..[[<a href="https://github.com/Twinzo1/openwrt/tree/master/luci-app-watchpig" target="_blank">]]
	..translate("Project address")
	..[[</a>]]
	..[[</strong><br />]])
-----------------------------Generate configuration----------------------------
s = m:section(TypedSection, "mac",translate("Clone Wan address"), translate("For the moment, Only support to modify WAN and WAN6 port"))
-- 这里对应config里面的option
s.addremove = false
s.anonymous = true  

s:tab("basic",translate("Generate configuration"))
os = s:taboption("basic",Value,"mac",translate(""))
os.template ="watchpigMac"

enable=s:taboption("basic",Flag,"enable",translate("Timing change Mac"))
enable.rmempty = false
enable.default=0

week=s:taboption("basic",ListValue,"week",translate("Week/Day"))
week:value(0,translate("Everyday"))
week:value(1,translate("Monday"))
week:value(2,translate("Tuesday"))
week:value(3,translate("Wednesday"))
week:value(4,translate("Thursday"))
week:value(5,translate("Friday"))
week:value(6,translate("Saturday"))
week:value(7,translate("Sunday"))
week.default=0
week.rmempty = true
week:depends({enable="1"})

hour=s:taboption("basic",Value,"hour",translate("Hour"))
hour.datatype = "range(0,23)"
hour.default=0
hour.rmempty = true
hour:depends({enable="1"})

pass=s:taboption("basic",Value,"minute",translate("Minute"))
pass.datatype = "range(0,59)"
pass.default=0
pass.rmempty = true
pass:depends({enable="1"})

reboot_run=s:taboption("basic",Flag,"reboot_run",translate("Modify mac at boot"))
reboot_run.rmempty = true
reboot_run.default=0
----------------------Log-------------------------------
s:tab("logger", translate("Log"))
s.anonymous=true
local a="/tmp/watchpig.log"
watchpig_log=s:taboption("logger",TextValue,"sylogtext")
watchpig_log.template="watchpigLog"
--------------------watchpig----------------------------
-- Copyright 2012 Christian Gagneraud <chris@techworks.ie>
-- Licensed to the public under the Apache License 2.0.
o = m:section(TypedSection, "watchpig",translate("Custom mac event(Modified on the original Watchcat)"),translate("Watchcat allows configuring a periodic reboot when the " ..
		  "Internet connection has been lost for a certain period of time."))
o.addremove = true
o.anonymous=true

mode = o:option(ListValue, "mode",
		translate("Operating mode"))
mode.default = "allways"
mode:value("allways",translate("Periodic reboot"))
mode:value("Noping",translate("Periodic reconnect"))--周期性重连
mode:value("RanMac",translate("Periodic Reconnect with modifying mac"))--周期性重连并修改mac
mode:value("ping1",translate("Reboot on internet connection lost"))
mode:value("ping2",translate("Reconnect on internet connection lost"))--断网重连
mode:value("ping3",translate("Reconnect with modifying mac on internet connection lost"))--断网重连并修改mac

forcedelay = o:option(Value, "forcedelay",
		      translate("Forced reboot/reconnect delay"),
		      translate("When rebooting the system, the watchcat will trigger a soft reboot. " ..
				"Entering a non zero value here will trigger a delayed hard reboot " ..
				"if the soft reboot fails. Enter a number of seconds to enable, " ..
				"use 0 to disable"))
forcedelay.datatype = "uinteger"
forcedelay.default = "0"

period = o:option(Value, "period", 
		  translate("Period"),
		  translate("In periodic mode, it defines the reboot period. " ..
			    "In internet mode, it defines the longest period of " .. 
			    "time without internet access before a reboot is engaged." ..
			    "Default unit is seconds, you can use the " ..
			    "suffix 'm' for minutes, 'h' for hours or 'd' " ..
			    "for days"))

pinghost = o:option(Value, "pinghosts", 
		    translate("Ping host"),
		    translate("Host address to ping"))
pinghost.datatype = "host(1)"
pinghost.default = "8.8.8.8"
pinghost:depends({mode="ping3"})
pinghost:depends({mode="ping2"})
pinghost:depends({mode="ping1"})

pingperiod = o:option(Value, "pingperiod", 
		      translate("Ping period"),
		      translate("How often to check internet connection. " ..
				"Default unit is seconds, you can you use the " ..
				"suffix 'm' for minutes, 'h' for hours or 'd' " ..
				"for days"))
pingperiod:depends({mode="ping3"})
pingperiod:depends({mode="ping2"})
pingperiod:depends({mode="ping1"})

custom = o:option(Value,"command",translate("Custom command"),
			translate("You can add a custom network event command here."))

local apply = luci.http.formvalue("cbi.apply")
local mac=luci.http.formvalue("c_mac")
if apply then
    luci.sys.exec("uci set watchpig.@mac[0].macaddr=%s" % mac)
    luci.sys.exec("uci commit watchpig")
end

return m

