#!/usr/bin/lua

require "nixio"
require "luci.util"
require "luci.sys"
local ntm = require "luci.model.network".init()
local type = arg[1]
local rv = {}

local wan = ntm:get_wannet()
local wan6 = ntm:get_wan6net()

if wan then
	rv.wan = {
		ipaddr  = wan:ipaddr(),
		gwaddr  = wan:gwaddr(),
		netmask = wan:netmask(),
		dns     = wan:dnsaddrs(),
		expires = wan:expires(),
		uptime  = wan:uptime(),
		proto   = wan:proto(),
		ifname  = wan:ifname()
	}
end

if wan6 then
	rv.wan6 = {
		ip6addr   = wan6:ip6addr(),
		gw6addr   = wan6:gw6addr(),
		dns       = wan6:dns6addrs(),
		ip6prefix = wan6:ip6prefix(),
		uptime    = wan6:uptime(),
		proto     = wan6:proto(),
		ifname    = wan6:ifname()
	}
end

if type == "dns" then
	if wan then
		if #(rv.wan.dns) >= 1 then
			for i=1, #(rv.wan.dns) do
				if rv.wan.dns[i] ~= rv.wan.gwaddr then
					print(rv.wan.dns[i])
				end
			end
		end
		print(rv.wan.gwaddr)
	end
end

if type == "dns6" then
	if wan6 then
		if #(rv.wan6.dns) >= 1 then
			for i=1, #(rv.wan6.dns) do
				if rv.wan6.dns[i] ~= rv.wan6.gw6addr then
					print(rv.wan6.dns[i])
				end
			end
		end
		print(rv.wan6.gw6addr)
	end
end