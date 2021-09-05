-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008-2015 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.tools.webadmin", package.seeall)

local util = require "luci.util"
local uci  = require "luci.model.uci"
local ip   = require "luci.ip"

function byte_format(byte)
	local suff = {"B", "KB", "MB", "GB", "TB"}
	for i=1, 5 do
		if byte > 1024 and i < 5 then
			byte = byte / 1024
		else
			return string.format("%.2f %s", byte, suff[i]) 
		end 
	end
end

function date_format(secs)
	local suff = {"min", "h", "d"}
	local mins = 0
	local hour = 0
	local days = 0
	
	secs = math.floor(secs)
	if secs > 60 then
		mins = math.floor(secs / 60)
		secs = secs % 60
	end
	
	if mins > 60 then
		hour = math.floor(mins / 60)
		mins = mins % 60
	end
	
	if hour > 24 then
		days = math.floor(hour / 24)
		hour = hour % 24
	end
	
	if days > 0 then
		return string.format("%.0fd %02.0fh %02.0fmin %02.0fs", days, hour, mins, secs)
	else
		return string.format("%02.0fh %02.0fmin %02.0fs", hour, mins, secs)
	end
end

function cbi_add_networks(field)
	uci.cursor():foreach("network", "interface",
		function (section)
			if section[".name"] ~= "loopback" then
				field:value(section[".name"])
			end
		end
	)
	field.titleref = luci.dispatcher.build_url("admin", "network", "network")
end

function cbi_add_knownips(field)
	local _, n
	for _, n in ipairs(ip.neighbors({ family = 4 })) do
		if n.dest then
			field:value(n.dest:string())
		end
	end
end

function firewall_find_zone(name)
	local find
	
	luci.model.uci.cursor():foreach("firewall", "zone", 
		function (section)
			if section.name == name then
				find = section[".name"]
			end
		end
	)
	
	return find
end

function iface_get_network(iface)
	local link = ip.link(tostring(iface))
	if link.master then
		iface = link.master
	end

	local cur = uci.cursor()
	local dump = util.ubus("network.interface", "dump", { })
	if dump then
		local _, net
		for _, net in ipairs(dump.interface) do
			if net.l3_device == iface or net.device == iface then
				-- cross check with uci to filter out @name style aliases
				local uciname = cur:get("network", net.interface, "ifname")
				if type(uciname) == "string" and uciname:sub(1,1) ~= "@" or uciname then
					return net.interface
				end
			end
		end
	end
end
