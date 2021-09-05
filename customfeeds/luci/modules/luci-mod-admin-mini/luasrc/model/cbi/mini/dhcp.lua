-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local uci = require "luci.model.uci".cursor()
local ipc = require "luci.ip"
local wa  = require "luci.tools.webadmin"
local fs  = require "nixio.fs"

m = Map("dhcp", "DHCP")

s = m:section(TypedSection, "dhcp", "DHCP-Server")
s.anonymous = true
s.addremove = false
s.dynamic = false

s:depends("interface", "lan")

enable = s:option(ListValue, "ignore", translate("enable"), "")
enable:value(0, translate("enable"))
enable:value(1, translate("disable"))

start = s:option(Value, "start", translate("First leased address"))
start.rmempty = true
start:depends("ignore", "0")


limit = s:option(Value, "limit", translate("Number of leased addresses"), "")
limit:depends("ignore", "0")

function limit.cfgvalue(self, section)
	local value = Value.cfgvalue(self, section)
	
	if value then
		return tonumber(value) + 1
	end 
end

function limit.write(self, section, value)
	value = tonumber(value) - 1
	return Value.write(self, section, value) 
end

limit.rmempty = true

time = s:option(Value, "leasetime")
time:depends("ignore", "0")
time.rmempty = true


local leasefn, leasefp, leases
uci:foreach("dhcp", "dnsmasq",
 function(section)
 	leasefn = section.leasefile
 end
) 
local leasefp = leasefn and fs.access(leasefn) and io.lines(leasefn)
if leasefp then
	leases = {}
	for lease in leasefp do
		table.insert(leases, luci.util.split(lease, " "))
	end
end

if leases then
	v = m:section(Table, leases, translate("Active Leases"))
	name = v:option(DummyValue, 4, translate("Hostname"))
	function name.cfgvalue(self, ...)
		local value = DummyValue.cfgvalue(self, ...)
		return (value == "*") and "?" or value
	end
	ip = v:option(DummyValue, 3, translate("<abbr title=\"Internet Protocol Version 4\">IPv4</abbr>-Address"))
	mac  = v:option(DummyValue, 2, translate("<abbr title=\"Media Access Control\">MAC</abbr>-Address"))
	ltime = v:option(DummyValue, 1, translate("Leasetime remaining"))
	function ltime.cfgvalue(self, ...)
		local value = DummyValue.cfgvalue(self, ...)
		return wa.date_format(os.difftime(tonumber(value), os.time()))
	end
end

s2 = m:section(TypedSection, "host", translate("Static Leases"))
s2.addremove = true
s2.anonymous = true
s2.template = "cbi/tblsection"

name = s2:option(Value, "name", translate("Hostname"))
mac = s2:option(Value, "mac", translate("<abbr title=\"Media Access Control\">MAC</abbr>-Address"))
ip = s2:option(Value, "ip", translate("<abbr title=\"Internet Protocol Version 4\">IPv4</abbr>-Address"))

ipc.neighbors({ family = 4 }, function(n)
	if n.mac and n.dest then
		ip:value(n.dest:string())
		mac:value(n.mac, "%s (%s)" %{ n.mac, n.dest:string() })
	end
end)

return m

