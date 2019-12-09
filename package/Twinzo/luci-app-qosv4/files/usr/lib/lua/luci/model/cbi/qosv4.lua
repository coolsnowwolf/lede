require("luci.tools.webadmin")

--[[
config 'qos_settings' 
 	option 'enable' '0'
	option 'UP' '100'
	option 'DOWN' '500'
	option qos_scheduler 1

config 'qos_ip' 
  option 'enable' '0' 
  option 'limit_ip' '192.168.1.5' 
  option 'UPLOADR' '2'
  option 'DOWNLOADR' '2'
	option 'UPLOADC' '15'
	option 'DOWNLOADC' '15'
	option 'UPLOADR2' '1'
	option 'UPLOADC2' '5'
	option 'DOWNLOADR2' '1'
	option 'DOWNLOADC2' '2'

config 'qos_nolimit_ip' 
  option 'enable' '0' 
  option 'limit_ip' '192.168.1.6' 

]]--

local sys = require "luci.sys"

m = Map("qosv4", translate("qosv4 title","QOSv4"),
                translate("qosv4 title desc","qosv4 title desc"))

s = m:section(TypedSection, "qos_settings", translate("qos goble setting","qos goble setting"))
s.anonymous = true
s.addremove = false

enable = s:option(Flag, "enable", translate("qos enable", "qos enable"))
enable.default = false
enable.optional = false
enable.rmempty = false

qos_scheduler = s:option(Flag, "qos_scheduler", translate("qos scheduler enable", "qos scheduler enable"),
                         translate("qos scheduler desc","qos scheduler desc"))
qos_scheduler.default = false
qos_scheduler.optional = false
qos_scheduler.rmempty = false


DOWN = s:option(Value, "DOWN", translate("DOWN speed","DOWN speed"),
          translate("DOWN speed desc","DOWN speed desc"))
DOWN.optional = false
DOWN.rmempty = false

UP = s:option(Value, "UP", translate("UP speed","UP speed"),
          translate("UP speed desc","UP speed desc"))
UP.optional = false
UP.rmempty = false

DOWNLOADR = s:option(Value, "DOWNLOADR", translate("DOWNLOADR speed","DOWNLOADR speed"))
DOWNLOADR.optional = false
DOWNLOADR.rmempty = false

UPLOADR = s:option(Value, "UPLOADR", translate("UPLOADR speed","UPLOADR speed"))
UPLOADR.optional = false
UPLOADR.rmempty = false

DOWNLOADR2 = s:option(Value, "DOWNLOADR2", translate("DOWNLOADR2 speed","DOWNLOADR2 speed"))
DOWNLOADR2.optional = false
DOWNLOADR2.rmempty = false

UPLOADR2 = s:option(Value, "UPLOADR2", translate("UPLOADR2 speed","UPLOADR2 speed"))
UPLOADR2.optional = false
UPLOADR2.rmempty = false

DOWNLOADC2 = s:option(Value, "DOWNLOADC2", translate("DOWNLOADC2 speed","DOWNLOADC2 speed"))
DOWNLOADC2.optional = false
DOWNLOADC2.rmempty = false

UPLOADC2 = s:option(Value, "UPLOADC2", translate("UPLOADC2 speed","UPLOADC2 speed"))
UPLOADC2.optional = false
UPLOADC2.rmempty = false





s = m:section(TypedSection, "qos_ip", translate("qos black ip","qos black ip"))
s.template = "cbi/tblsection"
s.anonymous = true
s.addremove = true

enable = s:option(Flag, "enable", translate("enable", "enable"))
enable.default = false
enable.optional = false
enable.rmempty = false



limit_ip = s:option(Value, "limit_ip", translate("limit_ip","limit_ip"))
limit_ip.rmempty = true
luci.tools.webadmin.cbi_add_knownips(limit_ip)


DOWNLOADC = s:option(Value, "DOWNLOADC", translate("DOWNLOADC speed","DOWNLOADC speed"))
DOWNLOADC.optional = false
DOWNLOADC.rmempty = false

UPLOADC = s:option(Value, "UPLOADC", translate("UPLOADC speed","UPLOADC speed"))
UPLOADC.optional = false
UPLOADC.rmempty = false

ip_prio = s:option(Value, "ip_prio", translate("ip prio","ip prio"),
translate("ip prio desc"," default 5 "))
ip_prio.optional = false
ip_prio.rmempty = false


s = m:section(TypedSection, "transmission_limit", translate("transmission limit","transmission limit"))
s.template = "cbi/tblsection"
s.anonymous = true
s.addremove = false

enable = s:option(Flag, "enable", translate("enable", "enable"))
enable.default = false
enable.optional = false
enable.rmempty = false

downlimit= s:option(Value, "downlimit", translate("downlimit speed","downlimit speed"))
downlimit.optional = false
downlimit.rmempty = false

uplimit= s:option(Value, "uplimit", translate("uplimit speed","uplimit speed"))
uplimit.optional = false
uplimit.rmempty = false


s = m:section(TypedSection, "qos_nolimit_ip", translate("qos white","qos white"))
s.template = "cbi/tblsection"
s.anonymous = true
s.addremove = true

enable = s:option(Flag, "enable", translate("enable", "enable"))
enable.default = false
enable.optional = false
enable.rmempty = false

nolimit_mac= s:option(Value, "nolimit_mac", translate("white mac","white mac"))
nolimit_mac.rmempty = true

nolimit_ip= s:option(Value, "nolimit_ip", translate("white ip","white ip"))
nolimit_ip.rmempty = true


luci.ip.neighbors(function(entry)
  nolimit_ip:value(entry["IP address"])
	nolimit_mac:value(
		entry["HW address"],
		entry["HW address"] .. " (" .. entry["IP address"] .. ")"
	)
end)

return m

