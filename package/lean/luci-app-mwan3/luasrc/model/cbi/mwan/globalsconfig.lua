-- Copyright 2017 Florian Eckert <fe@dev.tdt.de>
-- Licensed to the public under the GNU General Public License v2.

local net = require "luci.model.network".init()

local s, m, mask, rtmon, rtlookup, logging, loglevel

m = Map("mwan3", translate("MWAN - Globals"))

s = m:section(NamedSection, "globals", "globals", nil)

mask = s:option(
	Value,
	"mmx_mask",
	translate("Firewall mask"),
	translate("Enter value in hex, starting with <code>0x</code>"))
mask.datatype = "hex(4)"
mask.default = "0xff00"

logging = s:option(Flag,
	"logging",
	translate("Logging"),
	translate("Enables global firewall logging"))

loglevel = s:option(
	ListValue,
	"loglevel",
	translate("Loglevel"),
	translate("Firewall loglevel"))
loglevel.default = "notice"
loglevel:value("emerg", translate("Emergency"))
loglevel:value("alert", translate("Alert"))
loglevel:value("crit", translate("Critical"))
loglevel:value("error", translate("Error"))
loglevel:value("warning", translate("Warning"))
loglevel:value("notice", translate("Notice"))
loglevel:value("info", translate("Info"))
loglevel:value("debug", translate("Debug"))
loglevel:depends("logging", "1")

rtmon = s:option(
	Value,
	"rtmon_interval",
	translate("Update interval"),
	translate("How often should rtmon update the interface routing table"))
rtmon.datatype = "integer"
rtmon.default = "5"
rtmon:value("1", translatef("%d second", 1))
rtmon:value("3", translatef("%d seconds", 3))
rtmon:value("5", translatef("%d seconds", 5))
rtmon:value("7", translatef("%d seconds", 7))
rtmon:value("10", translatef("%d seconds", 10))

rtlookup = s:option(DynamicList,
	"rt_table_lookup",
	translate("Routing table lookup"),
	translate("Also scan this Routing table for connected networks"))
rtlookup.datatype = "integer"
rtlookup:value("1", translatef("Routing table %d", 1))
rtlookup:value("2", translatef("Routing table %d", 2))
rtlookup:value("220", translatef("Routing table %d", 220))

return m
