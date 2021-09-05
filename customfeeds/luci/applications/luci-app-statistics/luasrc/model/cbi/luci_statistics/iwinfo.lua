-- Copyright 2011 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local m, s, o

m = Map("luci_statistics",
	translate("Wireless iwinfo Plugin Configuration"),
	translate("The iwinfo plugin collects statistics about wireless signal strength, noise and quality."))

s = m:section(NamedSection, "collectd_iwinfo", "luci_statistics")

o = s:option(Flag, "enable", translate("Enable this plugin"))
o.default = 0

o = s:option(Value, "Interfaces", translate("Monitor interfaces"),
	translate("Leave unselected to automatically determine interfaces to monitor."))
o.template = "cbi/network_ifacelist"
o.widget   = "checkbox"
o.nocreate = true
o:depends("enable", 1)

o = s:option(Flag, "IgnoreSelected", translate("Monitor all except specified"))
o.default = 0
o:depends("enable", 1)

return m
