-- Copyright 2011 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.tools.status", package.seeall)

local uci = require "luci.model.uci".cursor()

local function dhcp_leases_common(family)
	local rv = { }
	local nfs = require "nixio.fs"
	local leasefile = "/tmp/dhcp.leases"

	uci:foreach("dhcp", "dnsmasq",
		function(s)
			if s.leasefile and nfs.access(s.leasefile) then
				leasefile = s.leasefile
				return false
			end
		end)

	local fd = io.open(leasefile, "r")
	if fd then
		while true do
			local ln = fd:read("*l")
			if not ln then
				break
			else
				local ts, mac, ip, name, duid = ln:match("^(%d+) (%S+) (%S+) (%S+) (%S+)")
				local expire = tonumber(ts) or 0
				if ts and mac and ip and name and duid then
					if family == 4 and not ip:match(":") then
						rv[#rv+1] = {
							expires  = (expire ~= 0) and os.difftime(expire, os.time()),
							macaddr  = mac,
							ipaddr   = ip,
							hostname = (name ~= "*") and name
						}
					elseif family == 6 and ip:match(":") then
						rv[#rv+1] = {
							expires  = (expire ~= 0) and os.difftime(expire, os.time()),
							ip6addr  = ip,
							duid     = (duid ~= "*") and duid,
							hostname = (name ~= "*") and name
						}
					end
				end
			end
		end
		fd:close()
	end

	local lease6file = "/tmp/hosts/odhcpd"
	uci:foreach("dhcp", "odhcpd",
		function(t)
			if t.leasefile and nfs.access(t.leasefile) then
				lease6file = t.leasefile
				return false
			end
		end)
	local fd = io.open(lease6file, "r")
	if fd then
		while true do
			local ln = fd:read("*l")
			if not ln then
				break
			else
				local iface, duid, iaid, name, ts, id, length, ip = ln:match("^# (%S+) (%S+) (%S+) (%S+) (-?%d+) (%S+) (%S+) (.*)")
				local expire = tonumber(ts) or 0
				if ip and iaid ~= "ipv4" and family == 6 then
					rv[#rv+1] = {
						expires  = (expire >= 0) and os.difftime(expire, os.time()),
						duid     = duid,
						ip6addr  = ip,
						hostname = (name ~= "-") and name
					}
				elseif ip and iaid == "ipv4" and family == 4 then
					local mac, mac1, mac2, mac3, mac4, mac5, mac6
					if duid and type(duid) == "string" then
						 mac1, mac2, mac3, mac4, mac5, mac6 = duid:match("^(%x%x)(%x%x)(%x%x)(%x%x)(%x%x)(%x%x)$")
					end
					if not (mac1 and mac2 and mac3 and mac4 and mac5 and mac6) then
						mac = "FF:FF:FF:FF:FF:FF"
					else
						mac = mac1..":"..mac2..":"..mac3..":"..mac4..":"..mac5..":"..mac6
					end
					rv[#rv+1] = {
						expires  = (expire >= 0) and os.difftime(expire, os.time()),
						macaddr  = duid,
						macaddr  = mac:lower(),
						ipaddr   = ip,
						hostname = (name ~= "-") and name
					}
				end
			end
		end
		fd:close()
	end

	return rv
end

function dhcp_leases()
	return dhcp_leases_common(4)
end

function dhcp6_leases()
	return dhcp_leases_common(6)
end

function wifi_networks()
	local rv = { }
	local ntm = require "luci.model.network".init()

	local dev
	for _, dev in ipairs(ntm:get_wifidevs()) do
		local rd = {
			up       = dev:is_up(),
			device   = dev:name(),
			name     = dev:get_i18n(),
			networks = { }
		}

		local net
		for _, net in ipairs(dev:get_wifinets()) do
			rd.networks[#rd.networks+1] = {
				name       = net:shortname(),
				link       = net:adminlink(),
				up         = net:is_up(),
				mode       = net:active_mode(),
				ssid       = net:active_ssid(),
				bssid      = net:active_bssid(),
				encryption = net:active_encryption(),
				frequency  = net:frequency(),
				channel    = net:channel(),
				signal     = net:signal(),
				quality    = net:signal_percent(),
				noise      = net:noise(),
				bitrate    = net:bitrate(),
				ifname     = net:ifname(),
				assoclist  = net:assoclist(),
				country    = net:country(),
				txpower    = net:txpower(),
				txpoweroff = net:txpower_offset(),
				disabled   = (dev:get("disabled") == "1" or
				             net:get("disabled") == "1")
			}
		end

		rv[#rv+1] = rd
	end

	return rv
end

function wifi_network(id)
	local ntm = require "luci.model.network".init()
	local net = ntm:get_wifinet(id)
	if net then
		local dev = net:get_device()
		if dev then
			return {
				id         = id,
				name       = net:shortname(),
				link       = net:adminlink(),
				up         = net:is_up(),
				mode       = net:active_mode(),
				ssid       = net:active_ssid(),
				bssid      = net:active_bssid(),
				encryption = net:active_encryption(),
				frequency  = net:frequency(),
				channel    = net:channel(),
				signal     = net:signal(),
				quality    = net:signal_percent(),
				noise      = net:noise(),
				bitrate    = net:bitrate(),
				ifname     = net:ifname(),
				assoclist  = net:assoclist(),
				country    = net:country(),
				txpower    = net:txpower(),
				txpoweroff = net:txpower_offset(),
				disabled   = (dev:get("disabled") == "1" or
				              net:get("disabled") == "1"),
				device     = {
					up     = dev:is_up(),
					device = dev:name(),
					name   = dev:get_i18n()
				}
			}
		end
	end
	return { }
end

function switch_status(devs)
	local dev
	local switches = { }
	for dev in devs:gmatch("[^%s,]+") do
		local ports = { }
		local swc = io.popen("swconfig dev %q show" % dev, "r")
		if swc then
			local l
			repeat
				l = swc:read("*l")
				if l then
					local port, up = l:match("port:(%d+) link:(%w+)")
					if port then
						local speed  = l:match(" speed:(%d+)")
						local duplex = l:match(" (%w+)-duplex")
						local txflow = l:match(" (txflow)")
						local rxflow = l:match(" (rxflow)")
						local auto   = l:match(" (auto)")

						ports[#ports+1] = {
							port   = tonumber(port) or 0,
							speed  = tonumber(speed) or 0,
							link   = (up == "up"),
							duplex = (duplex == "full"),
							rxflow = (not not rxflow),
							txflow = (not not txflow),
							auto   = (not not auto)
						}
					end
				end
			until not l
			swc:close()
		end
		switches[dev] = ports
	end
	return switches
end
