-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local sys = require"luci.sys"
local ip  = require "luci.ip"

m = Map("coovachilli")

-- tun
s1 = m:section(TypedSection, "tun")
s1.anonymous = true

s1:option( Flag, "usetap" )
s1:option( Value, "tundev" ).optional = true
s1:option( Value, "txqlen" ).optional = true

net = s1:option( Value, "net" )
for _, route in ipairs(ip.routes({ family = 4, type = 1 })) do
	if route.dest:prefix() > 0 and route.dest:prefix() < 32 then
		net:value( route.dest:string() )
	end
end

s1:option( Value, "dynip" ).optional = true
s1:option( Value, "statip" ).optional = true

s1:option( Value, "dns1" ).optional = true
s1:option( Value, "dns2" ).optional = true
s1:option( Value, "domain" ).optional = true

s1:option( Value, "ipup" ).optional = true
s1:option( Value, "ipdown" ).optional = true

s1:option( Value, "conup" ).optional = true
s1:option( Value, "condown" ).optional = true


-- dhcp config
s2 = m:section(TypedSection, "dhcp")
s2.anonymous = true

dif = s2:option( Value, "dhcpif" )
for _, nif in ipairs(sys.net.devices()) do
	if nif ~= "lo" then dif:value(nif) end
end

s2:option( Value, "dhcpmac" ).optional = true
s2:option( Value, "lease" ).optional = true
s2:option( Value, "dhcpstart" ).optional = true
s2:option( Value, "dhcpend" ).optional = true

s2:option( Flag, "eapolenable" )


return m
