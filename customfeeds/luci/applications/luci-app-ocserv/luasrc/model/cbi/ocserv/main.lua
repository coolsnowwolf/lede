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

local o_sha = s:taboption("general", DummyValue, "sha_hash", translate("Server's certificate SHA1 hash"),
			  translate("That value should be communicated to the client to verify the server's certificate"))
local o_pki = s:taboption("general", DummyValue, "pkid", translate("Server's Public Key ID"),
			  translate("An alternative value to be communicated to the client to verify the server's certificate; this value only depends on the public key"))

local fd = io.popen("/usr/bin/certtool -i --infile /etc/ocserv/server-cert.pem", "r")
if fd then local ln
	local found_sha = false
	local found_pki = false
	local complete = 0
	while complete < 2 do
		local ln = fd:read("*l")
		if not ln then
			break
		elseif ln:match("SHA%-?1 fingerprint:") then
			found_sha = true
		elseif found_sha then
			local hash = ln:match("([a-f0-9]+)")
			o_sha.default = hash and hash:upper()
			complete = complete + 1
			found_sha = false
		elseif ln:match("Public Key I[Dd]:") then
			found_pki = true
		elseif found_pki then
			local hash = ln:match("([a-f0-9]+)")
			o_pki.default = hash and "sha1:" .. hash:upper()
			complete = complete + 1
			found_pki = false
		end
	end
	fd:close()
end

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

s:taboption("general", Value, "port", translate("Port"),
	translate("The same UDP and TCP ports will be used"))
s:taboption("general", Value, "max_clients", translate("Max clients"))
s:taboption("general", Value, "max_same", translate("Max same clients"))
s:taboption("general", Value, "dpd", translate("Dead peer detection time (secs)"))

local pip = s:taboption("general", Flag, "predictable_ips", translate("Predictable IPs"),
	translate("The assigned IPs will be selected deterministically"))
pip.default = "1"

local compr = s:taboption("general", Flag, "compression", translate("Enable compression"),
	translate("Enable compression"))
compr.default = "0"

local udp = s:taboption("general", Flag, "udp", translate("Enable UDP"),
	translate("Enable UDP channel support; this must be enabled unless you know what you are doing"))
udp.default = "1"

local cisco = s:taboption("general", Flag, "cisco_compat", translate("AnyConnect client compatibility"),
	translate("Enable support for CISCO AnyConnect clients"))
cisco.default = "1"


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

--[[Networking options]]--

local parp = s:taboption("general", Flag, "proxy_arp", translate("Enable proxy arp"),
	translate("Provide addresses to clients from a subnet of LAN; if enabled the network below must be a subnet of LAN. Note that the first address of the specified subnet will be reserved by ocserv, so it should not be in use. If you have a network in LAN covering 192.168.1.0/24 use 192.168.1.192/26 to reserve the upper 62 addresses."))
parp.default = "0"

ipaddr = s:taboption("general", Value, "ipaddr", translate("VPN <abbr title=\"Internet Protocol Version 4\">IPv4</abbr>-Network-Address"),
        translate("The IPv4 subnet address to provide to clients; this should be some private network different than the LAN addresses unless proxy ARP is enabled. Leave empty to attempt auto-configuration."))
ipaddr.datatype = "ip4addr"
ipaddr.default = "192.168.100.1"

nm = s:taboption("general", Value, "netmask", translate("VPN <abbr title=\"Internet Protocol Version 4\">IPv4</abbr>-Netmask"),
        translate("The mask of the subnet above."))
nm.datatype = "ip4addr"
nm.default = "255.255.255.0"
nm:value("255.255.255.0")
nm:value("255.255.0.0")
nm:value("255.0.0.0")

if has_ipv6 then
	ip6addr = s:taboption("general", Value, "ip6addr", translate("VPN <abbr title=\"Internet Protocol Version 6\">IPv6</abbr>-Network-Address"), translate("<abbr title=\"Classless Inter-Domain Routing\">CIDR</abbr>-Notation: address/prefix"),
	                      translate("The IPv6 subnet address to provide to clients; leave empty to attempt auto-configuration."))
	ip6addr.datatype = "ip6addr"
end


--[[DNS]]--

s = m:section(TypedSection, "dns", translate("DNS servers"),
	translate("The DNS servers to be provided to clients; can be either IPv6 or IPv4. Typically you should include the address of this device"))
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

o = s:option(Value, "netmask", translate("Netmask (or IPv6-prefix)"))
o.default = "255.255.255.0"
o:value("255.255.255.0")
o:value("255.255.0.0")
o:value("255.0.0.0")


return m
