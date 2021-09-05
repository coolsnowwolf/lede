-- Licensed to the public under the Apache License 2.0.

require("luci.model.uci")

m = Map("luci_splash", translate("Client-Splash"), translate("Client-Splash is a hotspot authentification system for wireless mesh networks."))

s = m:section(NamedSection, "general", "core", translate("General"))
s.addremove = false

s:option(Value, "leasetime", translate("Clearance time"), translate("Clients that have accepted the splash are allowed to use the network for that many hours."))
local redir = s:option(Value, "redirect_url", translate("Redirect target"), translate("Clients are redirected to this page after they have accepted the splash. If this is left empty they are redirected to the page they had requested."))
redir.rmempty = true

s:option(Value, "limit_up", translate("Upload limit"), translate("Clients upload speed is limited to this value (kbyte/s)"))
s:option(Value, "limit_down", translate("Download limit"), translate("Clients download speed is limited to this value (kbyte/s)"))

s:option(DummyValue, "_tmp", "",
	translate("Bandwidth limit for clients is only activated when both up- and download limit are set. " ..
	"Use a value of 0 here to completely disable this limitation. Whitelisted clients are not limited."))

s = m:section(TypedSection, "iface", translate("Interfaces"), translate("Interfaces that are used for Splash."))

s.template = "cbi/tblsection"
s.addremove = true
s.anonymous = true

local uci = luci.model.uci.cursor()

zone = s:option(ListValue, "zone", translate("Firewall zone"),
	translate("Splash rules are integrated in this firewall zone"))

uci:foreach("firewall", "zone",
	function (section)
		zone:value(section.name)
	end)
	
iface = s:option(ListValue, "network", translate("Network"),
	translate("Intercept client traffic on this Interface"))

uci:foreach("network", "interface",
	function (section)
		if section[".name"] ~= "loopback" then
			iface:value(section[".name"])
		end
	end)
	
uci:foreach("network", "alias",
	function (section)
		iface:value(section[".name"])
	end)


s = m:section(TypedSection, "whitelist", translate("Whitelist"),
	translate("MAC addresses of whitelisted clients. These do not need to accept the splash and are not bandwidth limited."))

s.template = "cbi/tblsection"
s.addremove = true
s.anonymous = true
s:option(Value, "mac", translate ("MAC Address"))


s = m:section(TypedSection, "blacklist", translate("Blacklist"),
	translate("MAC addresses in this list are blocked."))

s.template = "cbi/tblsection"
s.addremove = true
s.anonymous = true
s:option(Value, "mac", translate ("MAC Address"))

s = m:section(TypedSection, "subnet", translate("Allowed hosts/subnets"),
	translate("Destination hosts and networks that are excluded from splashing, i.e. they are always allowed."))

s.template = "cbi/tblsection"
s.addremove = true
s.anonymous = true
s:option(Value, "ipaddr", translate("IP Address"))
s:option(Value, "netmask", translate("Netmask"), translate("optional when using host addresses")).rmempty = true
	
return m
