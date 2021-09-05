-- Copyright 2011 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local map, section, net = ...

local ipaddr, peeraddr, ip6addr, tunnelid, username, password
local defaultroute, metric, ttl, mtu


ipaddr = s:taboption("general", Value, "ipaddr",
	translate("Local IPv4 address"),
	translate("Leave empty to use the current WAN address"))

ipaddr.datatype = "ip4addr"


peeraddr = s:taboption("general", Value, "peeraddr",
	translate("Remote IPv4 address"),
	translate("This is usually the address of the nearest PoP operated by the tunnel broker"))

peeraddr.rmempty  = false
peeraddr.datatype = "ip4addr"


ip6addr = s:taboption("general", Value, "ip6addr",
	translate("Local IPv6 address"),
	translate("This is the local endpoint address assigned by the tunnel broker, it usually ends with <code>...:2/64</code>"))

ip6addr.datatype = "ip6addr"


local ip6prefix = s:taboption("general", Value, "ip6prefix",
	translate("IPv6 routed prefix"),
	translate("This is the prefix routed to you by the tunnel broker for use by clients"))

ip6prefix.datatype = "ip6addr"


local update = section:taboption("general", Flag, "_update",
	translate("Dynamic tunnel"),
	translate("Enable HE.net dynamic endpoint update"))

update.enabled  = "1"
update.disabled = "0"

function update.write() end
function update.remove() end
function update.cfgvalue(self, section)
	return (tonumber(m:get(section, "tunnelid") or "") ~= nil)
		and self.enabled or self.disabled
end


tunnelid = section:taboption("general", Value, "tunnelid", translate("Tunnel ID"))
tunnelid.datatype = "uinteger"
tunnelid:depends("_update", update.enabled)


username = section:taboption("general", Value, "username",
	translate("HE.net username"),
	translate("This is the plain username for logging into the account"))

username:depends("_update", update.enabled)
username.validate = function(self, val, sid)
	if type(val) == "string" and #val == 32 and val:match("^[a-fA-F0-9]+$") then
		return nil, translate("The HE.net endpoint update configuration changed, you must now use the plain username instead of the user ID!")
	end
	return val
end


password = section:taboption("general", Value, "password",
	translate("HE.net password"),
	translate("This is either the \"Update Key\" configured for the tunnel or the account password if no update key has been configured"))

password.password = true
password:depends("_update", update.enabled)


defaultroute = section:taboption("advanced", Flag, "defaultroute",
	translate("Default gateway"),
	translate("If unchecked, no default route is configured"))

defaultroute.default = defaultroute.enabled


metric = section:taboption("advanced", Value, "metric",
	translate("Use gateway metric"))

metric.placeholder = "0"
metric.datatype    = "uinteger"
metric:depends("defaultroute", defaultroute.enabled)


ttl = section:taboption("advanced", Value, "ttl", translate("Use TTL on tunnel interface"))
ttl.placeholder = "64"
ttl.datatype    = "range(1,255)"


mtu = section:taboption("advanced", Value, "mtu", translate("Use MTU on tunnel interface"))
mtu.placeholder = "1280"
mtu.datatype    = "max(9200)"
