-- Copyright 2018 Nick Peng (pymumu@gmail.com)

require("nixio.fs")
require("luci.http")

m = Map("smartdns", translate("SmartDNS"),
	translate("SmartDNS is a local dns server to find fastest ip."))

if luci.sys.call("pidof smartdns >/dev/null") == 0 then
	m = Map("smartdns", translate("SmartDNS"), "%s - %s" %{translate("SmartDNS"), translate("RUNNING")})
else
	m = Map("smartdns", translate("SmartDNS"), "%s - %s" %{translate("SmartDNS"), translate("NOT RUNNING")})
end

-- Basic
s = m:section(TypedSection, "smartdns", translate("Settings"), translate("General Settings"))
s.anonymous = true

---- Eanble
o = s:option(Flag, "enabled", translate("Enable"), translate("Enable or disable smartdns server"))
o.rempty      = false

---- Port
o = s:option(Value, "port", translate("Local Port"), translate("Smartdns local server port"))
o.placeholder = 5353
o.default     = 5353
o.datatype    = "port"
o.rempty      = false

o = s:option(Flag, "redirect", translate("redirect"), translate("redirect standard dns query from 53 to smartdns"))
o.default     = true
o.placeholder = "1"
o.rempty      = false

---- cache-size
o = s:option(Value, "cache_size", translate("Cache Size"), translate("DNS domain result cache size"))
o.rempty      = true

---- rr-ttl
o = s:option(Value, "rr_ttl", translate("Domain TTL"), translate("TTL for all domain result."))
o.rempty      = true

---- rr-ttl-min
o = s:option(Value, "rr_ttl_min", translate("Domain TTL Min"), translate("Minimum TTL for all domain result."))
o.rempty      = true

---- rr-ttl-max
o = s:option(Value, "rr_ttl_max", translate("Domain TTL Max"), translate("Maximum TTL for all domain result."))
o.rempty      = true

-- Upstream servers
s = m:section(TypedSection, "server", translate("Upstream Servers"), translate("Upstream Servers, support UDP, TCP protocol"))
s.anonymous = true
s.addremove = true
s.template = "cbi/tblsection"

---- name
s:option(Value, "name", translate("Name"), translate("DNS Server name"))
---- IP address
o = s:option(Value, "ip", translate("ip"), translate("DNS Server ip"))
o.datatype = "ipaddr"
o.rmempty = false 
---- port
o = s:option(Value, "port", translate("port"), translate("DNS Server port"))
o.placeholder = 53
o.default     = 53
o.datatype    = "port"
o.rempty      = false
---- type
o = s:option(ListValue, "type", translate("type"), translate("DNS Server type"))
o.placeholder = "udp"
o:value("udp", translate("udp"))
o:value("tcp", translate("tcp"))
o.default     = "udp"
o.rempty      = false

-- Doman addresss
s = m:section(TypedSection, "smartdns", translate("Domain Address"), 
	translate("DNS Server name"))
s.anonymous = true

---- address
addr = s:option(Value, "address",
	translate(""), 
	translate("Specify an IP address to return for any host in the given domains " ..
	  "Queries in the domains are never forwarded and always replied to with the specified IP address which may be IPv4 or IPv6. " ..
	  ""))

addr.template = "cbi/tvalue"
addr.rows = 20

function addr.cfgvalue(self, section)
	return nixio.fs.readfile("/etc/smartdns/address.conf")
end

function addr.write(self, section, value)
	value = value:gsub("\r\n?", "\n")
	nixio.fs.writefile("/etc/smartdns/address.conf", value)
end

-- Doman addresss
s = m:section(TypedSection, "smartdns", translate("Technical Support"), 
	translate("Technical Support"))
s.anonymous = true

o = s:option(Button, "web")
o.title = translate("SmartDNS official website")
o.inputtitle = translate("open website")
o.inputstyle = "apply"
o.write = function()
	luci.http.redirect("https://pymumu.github.io/smartdns")
end

o = s:option(Button, "Donate")
o.title = translate("Donate to smartdns")
o.inputtitle = translate("Donate")
o.inputstyle = "apply"
o.write = function()
	luci.http.redirect("https://pymumu.github.io/smartdns/#donate")
end

return m

