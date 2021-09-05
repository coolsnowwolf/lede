-- Copyright 2011 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local map, section, net = ...

local device, apn, service, pincode, username, password
local ipv6, maxwait, defaultroute, metric, peerdns, dns,
      keepalive_failure, keepalive_interval, demand


mca = s:taboption("ahcp", Value, "multicast_address", translate("Multicast address"))
mca.optional    = true
mca.placeholder = "ff02::cca6:c0f9:e182:5359"
mca.datatype    = "ip6addr"
mca:depends("proto", "ahcp")

port = s:taboption("ahcp", Value, "port", translate("Port"))
port.optional    = true
port.placeholder = 5359
port.datatype    = "port"
port:depends("proto", "ahcp")

fam = s:taboption("ahcp", ListValue, "_family", translate("Protocol family"))
fam:value("", translate("IPv4 and IPv6"))
fam:value("ipv4", translate("IPv4 only"))
fam:value("ipv6", translate("IPv6 only"))
fam:depends("proto", "ahcp")

function fam.cfgvalue(self, section)
	local v4 = m.uci:get_bool("network", section, "ipv4_only")
	local v6 = m.uci:get_bool("network", section, "ipv6_only")
	if v4 then
		return "ipv4"
	elseif v6 then
		return "ipv6"
	end
	return ""
end

function fam.write(self, section, value)
	if value == "ipv4" then
		m.uci:set("network", section, "ipv4_only", "true")
		m.uci:delete("network", section, "ipv6_only")
	elseif value == "ipv6" then
		m.uci:set("network", section, "ipv6_only", "true")
		m.uci:delete("network", section, "ipv4_only")
	end
end

function fam.remove(self, section)
	m.uci:delete("network", section, "ipv4_only")
	m.uci:delete("network", section, "ipv6_only")
end

nodns = s:taboption("ahcp", Flag, "no_dns", translate("Disable DNS setup"))
nodns.optional = true
nodns.enabled  = "true"
nodns.disabled = "false"
nodns.default  = nodns.disabled
nodns:depends("proto", "ahcp")

ltime = s:taboption("ahcp", Value, "lease_time", translate("Lease validity time"))
ltime.optional    = true
ltime.placeholder = 3666
ltime.datatype    = "uinteger"
ltime:depends("proto", "ahcp")

