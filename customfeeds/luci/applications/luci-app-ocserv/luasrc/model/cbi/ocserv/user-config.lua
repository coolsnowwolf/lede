-- Copyright 2014 Nikos Mavrogiannopoulos <n.mavrogiannopoulos@gmail.com>
-- Licensed to the public under the Apache License 2.0.

local fs = require "nixio.fs"
local has_ipv6 = fs.access("/proc/net/ipv6_route")

m = Map("ocserv", translate("OpenConnect VPN"))

s = m:section(TypedSection, "ocserv", "OpenConnect")
s.anonymous = true

s:tab("general",  translate("General Settings"))
s:tab("ca", translate("CA certificate"))
s:tab("template", translate("Edit Template"))

local e = s:taboption("general", Flag, "enable", translate("Enable server"))
e.rmempty = false
e.default = "1"

function m.on_commit(map)
	luci.sys.call("/usr/bin/occtl reload  >/dev/null 2>&1")
end

function e.write(self, section, value)
	if value == "0" then
		luci.sys.call("/etc/init.d/ocserv stop >/dev/null 2>&1")
		luci.sys.call("/etc/init.d/ocserv disable  >/dev/null 2>&1")
	else
		luci.sys.call("/etc/init.d/ocserv enable  >/dev/null 2>&1")
		luci.sys.call("/etc/init.d/ocserv restart  >/dev/null 2>&1")
	end
	Flag.write(self, section, value)
end

local o

o = s:taboption("general", ListValue, "auth", translate("User Authentication"),
	translate("The authentication method for the users. The simplest is plain with a single username-password pair. Use PAM modules to authenticate using another server (e.g., LDAP, Radius)."))
o.rmempty = false
o.default = "plain"
o:value("plain")
o:value("PAM")

o = s:taboption("general", Value, "zone", translate("Firewall Zone"),
	translate("The firewall zone that the VPN clients will be set to"))
o.nocreate = true
o.default = "lan"
o.template = "cbi/firewall_zonelist"

s:taboption("general", Value, "port", translate("Port"),
	translate("The same UDP and TCP ports will be used"))
s:taboption("general", Value, "max_clients", translate("Max clients"))
s:taboption("general", Value, "max_same", translate("Max same clients"))
s:taboption("general", Value, "dpd", translate("Dead peer detection time (secs)"))

local pip = s:taboption("general", Flag, "predictable_ips", translate("Predictable IPs"),
	translate("The assigned IPs will be selected deterministically"))
pip.default = "1"

local udp = s:taboption("general", Flag, "udp", translate("Enable UDP"),
	translate("Enable UDP channel support; this must be enabled unless you know what you are doing"))
udp.default = "1"

local cisco = s:taboption("general", Flag, "cisco_compat", translate("AnyConnect client compatibility"),
	translate("Enable support for CISCO AnyConnect clients"))
cisco.default = "1"

ipaddr = s:taboption("general", Value, "ipaddr", translate("VPN <abbr title=\"Internet Protocol Version 4\">IPv4</abbr>-Network-Address"))
ipaddr.default = "192.168.100.1"
ipaddr.datatype = "ip4addr"

nm = s:taboption("general", Value, "netmask", translate("VPN <abbr title=\"Internet Protocol Version 4\">IPv4</abbr>-Netmask"))
nm.default = "255.255.255.0"
nm.datatype = "ip4addr"
nm:value("255.255.255.0")
nm:value("255.255.0.0")
nm:value("255.0.0.0")

if has_ipv6 then
	ip6addr = s:taboption("general", Value, "ip6addr", translate("VPN <abbr title=\"Internet Protocol Version 6\">IPv6</abbr>-Network-Address"), translate("<abbr title=\"Classless Inter-Domain Routing\">CIDR</abbr>-Notation: address/prefix"))
end


tmpl = s:taboption("template", Value, "_tmpl",
	translate("Edit the template that is used for generating the ocserv configuration."))

tmpl.template = "cbi/tvalue"
tmpl.rows = 20

function tmpl.cfgvalue(self, section)
	return nixio.fs.readfile("/etc/ocserv/ocserv.conf.template")
end

function tmpl.write(self, section, value)
	value = value:gsub("\r\n?", "\n")
	nixio.fs.writefile("/etc/ocserv/ocserv.conf.template", value)
end

ca = s:taboption("ca", Value, "_ca",
	translate("View the CA certificate used by this server. You will need to save it as 'ca.pem' and import it into the clients."))

ca.template = "cbi/tvalue"
ca.rows = 20

function ca.cfgvalue(self, section)
	return nixio.fs.readfile("/etc/ocserv/ca.pem")
end

--[[DNS]]--

s = m:section(TypedSection, "dns", translate("DNS servers"),
	translate("The DNS servers to be provided to clients; can be either IPv6 or IPv4"))
s.anonymous = true
s.addremove = true
s.template = "cbi/tblsection"

s:option(Value, "ip", translate("IP Address")).rmempty = true
s.datatype = "ipaddr"

--[[Routes]]--

s = m:section(TypedSection, "routes", translate("Routing table"),
	translate("The routing table to be provided to clients; you can mix IPv4 and IPv6 routes, the server will send only the appropriate. Leave empty to set a default route"))
s.anonymous = true
s.addremove = true
s.template = "cbi/tblsection"

s:option(Value, "ip", translate("IP Address")).rmempty = true
s.datatype = "ipaddr"

o = s:option(Value, "netmask", translate("Netmask (or IPv6-prefix)"))
o.default = "255.255.255.0"

o:value("255.255.255.0")
o:value("255.255.0.0")
o:value("255.0.0.0")


return m
