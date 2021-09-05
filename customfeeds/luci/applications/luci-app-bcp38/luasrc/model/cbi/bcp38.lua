--[[
LuCI - Lua Configuration Interface

Copyright 2014 Toke Høiland-Jørgensen <toke@toke.dk>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id$
]]--

local wa = require "luci.tools.webadmin"
local net = require "luci.model.network".init()
local ifaces = net:get_interfaces()

m = Map("bcp38", translate("BCP38"),
	translate("This function blocks packets with private address destinations " ..
		"from going out onto the internet as per " ..
		"<a href=\"http://tools.ietf.org/html/bcp38\">BCP 38</a>. " ..
		"For IPv6, only source specific default routes are installed, so " ..
		"no BCP38 firewall routes are needed."))

s = m:section(TypedSection, "bcp38", translate("BCP38 config"))
s.anonymous = true
-- BASIC
e = s:option(Flag, "enabled", translate("Enable"))
e.rmempty = false

a = s:option(Flag, "detect_upstream", translate("Auto-detect upstream IP"),
				translate("Attempt to automatically detect if the upstream IP " ..
					"will be blocked by the configuration, and add an exception if it will. " ..
					"If this does not work correctly, you can add exceptions manually below."))
a.rmempty = false

n = s:option(ListValue, "interface", translate("Interface name"), translate("Interface to apply the blocking to " ..
							"(should be the upstream WAN interface)."))
for _, iface in ipairs(ifaces) do
     if iface:is_up() then
	n:value(iface:name())
     end
end
n.rmempty = false

ma = s:option(DynamicList, "match",
	translate("Blocked IP ranges"))

ma.datatype = "ip4addr"

nm = s:option(DynamicList, "nomatch",
	translate("Allowed IP ranges"), translate("Takes precedence over blocked ranges. "..
						  "Use to whitelist your upstream network if you're behind a double NAT " ..
						  "and the auto-detection doesn't work."))

nm.datatype = "ip4addr"


return m
