-- Copyright 2011 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

m = Map("ahcpd", translate("AHCP Server"), translate("AHCP is an autoconfiguration protocol " ..
	"for IPv6 and dual-stack IPv6/IPv4 networks designed to be used in place of router " ..
	"discovery or DHCP on networks where it is difficult or impossible to configure a " ..
	"server within every link-layer broadcast domain, for example mobile ad-hoc networks."))


m:section(SimpleSection).template = "ahcp_status"

s = m:section(TypedSection, "ahcpd")
s:tab("general", translate("General Setup"))
s:tab("advanced", translate("Advanced Settings"))
s.addremove = false
s.anonymous = true


mode = s:taboption("general", ListValue, "mode", translate("Operation mode"))
mode:value("server", translate("Server"))
mode:value("forwarder", translate("Forwarder"))

net = s:taboption("general", Value, "interface", translate("Served interfaces"))
net.template = "cbi/network_netlist"
net.widget   = "checkbox"
net.nocreate = true

function net.cfgvalue(self, section)
	return m.uci:get("ahcpd", section, "interface")
end

pfx = s:taboption("general", DynamicList, "prefix", translate("Announced prefixes"),
	translate("Specifies the announced IPv4 and IPv6 network prefixes in CIDR notation"))
pfx.optional  = true
pfx.datatype  = "ipaddr"
pfx:depends("mode", "server")

nss = s:taboption("general", DynamicList, "name_server", translate("Announced DNS servers"),
	translate("Specifies the announced IPv4 and IPv6 name servers"))
nss.optional = true
nss.datatype = "ipaddr"
nss:depends("mode", "server")

ntp = s:taboption("general", DynamicList, "ntp_server", translate("Announced NTP servers"),
	translate("Specifies the announced IPv4 and IPv6 NTP servers"))
ntp.optional = true
ntp.datatype = "ipaddr"
ntp:depends("mode", "server")

mca = s:taboption("general", Value, "multicast_address", translate("Multicast address"))
mca.optional    = true
mca.placeholder = "ff02::cca6:c0f9:e182:5359"
mca.datatype    = "ip6addr"

port = s:taboption("general", Value, "port", translate("Port"))
port.optional    = true
port.placeholder = 5359
port.datatype    = "port"

fam = s:taboption("general", ListValue, "_family", translate("Protocol family"))
fam:value("", translate("IPv4 and IPv6"))
fam:value("ipv4", translate("IPv4 only"))
fam:value("ipv6", translate("IPv6 only"))

function fam.cfgvalue(self, section)
	local v4 = m.uci:get_bool("ahcpd", section, "ipv4_only")
	local v6 = m.uci:get_bool("ahcpd", section, "ipv6_only")
	if v4 then
		return "ipv4"
	elseif v6 then
		return "ipv6"
	end
	return ""
end

function fam.write(self, section, value)
	if value == "ipv4" then
		m.uci:set("ahcpd", section, "ipv4_only", "true")
		m.uci:delete("ahcpd", section, "ipv6_only")
	elseif value == "ipv6" then
		m.uci:set("ahcpd", section, "ipv6_only", "true")
		m.uci:delete("ahcpd", section, "ipv4_only")
	end
end

function fam.remove(self, section)
	m.uci:delete("ahcpd", section, "ipv4_only")
	m.uci:delete("ahcpd", section, "ipv6_only")
end

ltime = s:taboption("general", Value, "lease_time", translate("Lease validity time"))
ltime.optional    = true
ltime.placeholder = 3666
ltime.datatype    = "uinteger"


ld = s:taboption("advanced", Value, "lease_dir", translate("Lease directory"))
ld.datatype    = "directory"
ld.placeholder = "/var/lib/leases"

id = s:taboption("advanced", Value, "id_file", translate("Unique ID file"))
--id.datatype    = "file"
id.placeholder = "/var/lib/ahcpd-unique-id"

log = s:taboption("advanced", Value, "log_file", translate("Log file"))
--log.datatype    = "file"
log.placeholder = "/var/log/ahcpd.log"


return m
