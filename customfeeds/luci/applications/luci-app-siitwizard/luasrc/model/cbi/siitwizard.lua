-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008-2015 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local uci = require "luci.model.uci".cursor()
local bit = require "nixio".bit
local ip  = require "luci.ip"

-------------------- Init --------------------

--
-- Find link-local address
--
function find_ll()
	local _, r
	for _, r in ipairs(ip.routes({ family = 6, dest = "fe80::/64" })) do
		if r.dest:higher("fe80:0:0:0:ff:fe00:0:0") then
			return (r.dest - "fe80::")
		end
	end
	return ip.IPv6("::")
end

--
-- Determine defaults
--
local ula_prefix  = uci:get("siit", "ipv6", "ula_prefix")  or "fd00::"
local ula_global  = uci:get("siit", "ipv6", "ula_global")  or "00ca:ffee:babe::"		-- = Freifunk
local ula_subnet  = uci:get("siit", "ipv6", "ula_subnet")  or "0000:0000:0000:4223::"	-- = Berlin
local siit_prefix = uci:get("siit", "ipv6", "siit_prefix") or "::ffff:0000:0000"
local ipv4_pool   = uci:get("siit", "ipv4", "pool")        or "172.16.0.0/12"
local ipv4_netsz  = uci:get("siit", "ipv4", "netsize")     or "24"

--
-- Find IPv4 allocation pool
--
local gv4_net = ip.IPv4(ipv4_pool)

--
-- Generate ULA
--
local ula = ip.IPv6("::/64")

for _, prefix in ipairs({ ula_prefix, ula_global, ula_subnet }) do
	ula = ula:add(ip.IPv6(prefix))
end

ula = ula:add(find_ll())


-------------------- View --------------------
f = SimpleForm("siitwizward", "SIIT-Wizzard",
 "This wizzard helps to setup SIIT (IPv4-over-IPv6) translation according to RFC2765.")

f:field(DummyValue, "info_ula", "Mesh ULA address").value = ula:string()

f:field(DummyValue, "ipv4_pool", "IPv4 allocation pool").value =
	"%s (%i hosts)" %{ gv4_net:string(), 2 ^ ( 32 - gv4_net:prefix() ) - 2 }

f:field(DummyValue, "ipv4_size", "IPv4 LAN network prefix").value =
	"%i bit (%i hosts)" %{ ipv4_netsz, 2 ^ ( 32 - ipv4_netsz ) - 2 }

mode = f:field(ListValue, "mode", "Operation mode")
mode:value("client", "Client")
mode:value("gateway", "Gateway")

dev = f:field(ListValue, "device", "Wireless device")
uci:foreach("wireless", "wifi-device",
	function(section)
		dev:value(section[".name"])
	end)

lanip = f:field(Value, "ipaddr", "LAN IPv4 subnet")
function lanip.formvalue(self, section)
	local val = self.map:formvalue(self:cbid(section))
	local net = ip.IPv4("%s/%i" %{ val, ipv4_netsz })

	if net then
		if gv4_net:contains(net) then
			if not net:minhost():equal(net:host()) then
				self.error = { [section] = true }
				f.errmessage = "IPv4 address is not the first host of " ..
					"subnet, expected " .. net:minhost():string()
			end
		else
			self.error = { [section] = true }
			f.errmessage = "IPv4 address is not within the allocation pool"
		end
	else
		self.error = { [section] = true }
		f.errmessage = "Invalid IPv4 address given"
	end

	return val
end

dns = f:field(Value, "dns", "DNS server for LAN clients")
dns.value = "141.1.1.1"

-------------------- Control --------------------
function f.handle(self, state, data)
	if state == FORM_VALID then
		luci.http.redirect(luci.dispatcher.build_url("admin", "uci", "changes"))
		return false
	end
	return true
end

function mode.write(self, section, value)

	--
	-- Find LAN IPv4 range
	--
	local lan_net = ip.IPv4(
		( lanip:formvalue(section) or "172.16.0.1" ) .. "/" .. ipv4_netsz
	)

	if not lan_net then return end

	--
	-- Find wifi interface, dns server and hostname
	--
	local device      = dev:formvalue(section)
	local dns_server  = dns:formvalue(section) or "141.1.1.1"
	local hostname    = "siit-" .. lan_net:host():string():gsub("%.","-")

	--
	-- Configure wifi device
	--
	local wifi_device  = dev:formvalue(section)
	local wifi_essid   = uci:get("siit", "wifi", "essid")   or "6mesh.freifunk.net"
	local wifi_bssid   = uci:get("siit", "wifi", "bssid")   or "02:ca:ff:ee:ba:be"
	local wifi_channel = uci:get("siit", "wifi", "channel") or "1"

	-- nuke old device definition
	uci:delete_all("wireless", "wifi-iface",
		function(s) return s.device == wifi_device end )

	uci:delete_all("network", "interface",
		function(s) return s['.name'] == wifi_device end )

	-- create wifi device definition
	uci:tset("wireless", wifi_device, {
		disabled  = 0,
		channel   = wifi_channel,
--		txantenna = 1,
--		rxantenna = 1,
--		diversity = 0
	})

	uci:section("wireless", "wifi-iface", nil, {
		encryption = "none",
		mode       = "adhoc",
		txpower    = 10,
		sw_merge   = 1,
		network    = wifi_device,
		device     = wifi_device,
		ssid       = wifi_essid,
		bssid      = wifi_bssid,
	})

	--
	-- Gateway mode
	--
	--	* wan port is dhcp, lan port is 172.23.1.1/24
	--	* siit0 gets a dummy address: 169.254.42.42
	--	* wl0 gets an ipv6 address, in this case the fdca:ffee:babe::1:1/64
	--	* we do a ::ffff:ffff:0/96 route into siit0, so everything from 6mesh goes into translation.
	--	* an HNA6 of ::ffff:ffff:0:0/96 announces the mapped 0.0.0.0/0 ipv4 space.
	--	* MTU on WAN, LAN down to 1400, ipv6 headers are slighly larger.

	if value == "gateway" then

		-- wan mtu
		uci:set("network", "wan", "mtu", 1240)

		-- lan settings
		uci:tset("network", "lan", {
			mtu     = 1240,
			ipaddr  = lan_net:host():string(),
			netmask = lan_net:mask():string(),
			proto   = "static"
		})

		-- use full siit subnet
		siit_route = ip.IPv6(siit_prefix .. "/96")

		-- v4 <-> siit route
		uci:delete_all("network", "route",
			function(s) return s.interface == "siit0" end)

		uci:section("network", "route", nil, {
			interface = "siit0",
			target    = gv4_net:network():string(),
			netmask   = gv4_net:mask():string()
		})

	--
	-- Client mode
	--
	--	* 172.23.2.1/24 on its lan, fdca:ffee:babe::1:2 on wl0 and the usual dummy address on siit0.
	--	* we do a ::ffff:ffff:172.13.2.0/120 to siit0, because in this case, only traffic directed to clients needs to go into translation.
	--	* same route as HNA6 announcement to catch the traffic out of the mesh.
	--	* Also, MTU on LAN reduced to 1400.

	else

		-- lan settings
		uci:tset("network", "lan", {
			mtu     = 1240,
			ipaddr  = lan_net:host():string(),
			netmask = lan_net:mask():string()
		})

		-- derive siit subnet from lan config
		siit_route = ip.IPv6(
			siit_prefix .. "/" .. (96 + lan_net:prefix())
		):add(lan_net[2])

		-- ipv4 <-> siit route
		uci:delete_all("network", "route",
			function(s) return s.interface == "siit0" end)

		-- XXX: kind of a catch all, gv4_net would be better
		--      but does not cover non-local v4 space
		uci:section("network", "route", nil, {
			interface = "siit0",
			target    = "0.0.0.0",
			netmask   = "0.0.0.0"
		})
	end

	-- setup the firewall
	uci:delete_all("firewall", "zone",
		function(s) return (
			s['.name'] == "siit0" or s.name == "siit0" or
			s.network == "siit0" or	s['.name'] == wifi_device or
			s.name == wifi_device or s.network == wifi_device
		) end)

	uci:delete_all("firewall", "forwarding",
		function(s) return (
			s.src == wifi_device and s.dest == "siit0" or
			s.dest == wifi_device and s.src == "siit0" or
			s.src == "lan" and s.dest == "siit0" or
			s.dest == "lan" and s.src == "siit0"
		) end)

	uci:section("firewall", "zone", "siit0", {
		name    = "siit0",
		network = "siit0",
		input   = "ACCEPT",
		output  = "ACCEPT",
		forward = "ACCEPT"
	})

	uci:section("firewall", "zone", wifi_device, {
		name    = wifi_device,
		network = wifi_device,
		input   = "ACCEPT",
		output  = "ACCEPT",
		forward = "ACCEPT"
	})

	uci:section("firewall", "forwarding", nil, {
		src  = wifi_device,
		dest = "siit0"
	})

	uci:section("firewall", "forwarding", nil, {
		src  = "siit0",
		dest = wifi_device
	})

	uci:section("firewall", "forwarding", nil, {
		src  = "lan",
		dest = "siit0"
	})

	uci:section("firewall", "forwarding", nil, {
		src  = "siit0",
		dest = "lan"
	})

	-- firewall include
	uci:delete_all("firewall", "include",
		function(s) return s.path == "/etc/firewall.user" end)

	uci:section("firewall", "include", nil, {
		path = "/etc/firewall.user"
	})


	-- siit0 interface
	uci:delete_all("network", "interface",
		function(s) return ( s.ifname == "siit0" ) end)

	uci:section("network", "interface", "siit0", {
		ifname  = "siit0",
		proto   = "none"
	})

	-- siit0 route
	uci:delete_all("network", "route6",
		function(s) return siit_route:contains(ip.IPv6(s.target)) end)

	uci:section("network", "route6", nil, {
		interface = "siit0",
		target    = siit_route:string()
	})

	-- create wifi network interface
	uci:section("network", "interface", wifi_device, {
		proto   = "static",
		mtu     = 1400,
		ip6addr = ula:string()
	})

	-- nuke old olsrd interfaces
	uci:delete_all("olsrd", "Interface",
		function(s) return s.interface == wifi_device end)

	-- configure olsrd interface
	uci:foreach("olsrd", "olsrd",
		function(s) uci:set("olsrd", s['.name'], "IpVersion", 6) end)

	uci:section("olsrd", "Interface", nil, {
		ignore      = 0,
		interface   = wifi_device,
		Ip6AddrType = "unique-local"
	})

	-- hna6
	uci:delete_all("olsrd", "Hna6",
		function(s) return true end)

	uci:section("olsrd", "Hna6", nil, {
		netaddr = siit_route:host():string(),
		prefix  = siit_route:prefix()
	})

	-- txtinfo v6 & olsrd nameservice
	uci:foreach("olsrd", "LoadPlugin",
		function(s)
			if s.library == "olsrd_txtinfo.so.0.1" then
				uci:set("olsrd", s['.name'], "accept", "::1")
			elseif s.library == "olsrd_nameservice.so.0.3" then
				uci:set("olsrd", s['.name'], "name", hostname)
			end
		end)

	-- lan dns
	uci:tset("dhcp", "lan", {
		dhcp_option = "6," .. dns_server,
		start       = bit.band(lan_net:minhost():add(1)[2][2], 0xFF),
		limit       = ( 2 ^ ( 32 - lan_net:prefix() ) ) - 3
	})

	-- hostname
	uci:foreach("system", "system",
		function(s)
			uci:set("system", s['.name'], "hostname", hostname)
		end)

	uci:save("wireless")
	uci:save("firewall")
	uci:save("network")
	uci:save("system")
	uci:save("olsrd")
	uci:save("dhcp")
end

return f
