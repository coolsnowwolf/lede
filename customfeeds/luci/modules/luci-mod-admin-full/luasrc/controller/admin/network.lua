-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2011-2015 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.admin.network", package.seeall)

function index()
	local uci = require("luci.model.uci").cursor()
	local page

	page = node("admin", "network")
	page.target = firstchild()
	page.title  = _("Network")
	page.order  = 50
	page.index  = true

--	if page.inreq then
		local has_switch = false

		uci:foreach("network", "switch",
			function(s)
				has_switch = true
				return false
			end)

		if has_switch then
			page  = node("admin", "network", "vlan")
			page.target = cbi("admin_network/vlan")
			page.title  = _("Switch")
			page.order  = 20

			page = entry({"admin", "network", "switch_status"}, call("switch_status"), nil)
			page.leaf = true
		end


		local has_wifi = false

		uci:foreach("wireless", "wifi-device",
			function(s)
				has_wifi = true
				return false
			end)

		if has_wifi then
			page = entry({"admin", "network", "wireless_join"}, post("wifi_join"), nil)
			page.leaf = true

			page = entry({"admin", "network", "wireless_add"}, post("wifi_add"), nil)
			page.leaf = true

			page = entry({"admin", "network", "wireless_delete"}, post("wifi_delete"), nil)
			page.leaf = true

			page = entry({"admin", "network", "wireless_status"}, call("wifi_status"), nil)
			page.leaf = true

			page = entry({"admin", "network", "wireless_reconnect"}, post("wifi_reconnect"), nil)
			page.leaf = true

			page = entry({"admin", "network", "wireless_shutdown"}, post("wifi_shutdown"), nil)
			page.leaf = true

			page = entry({"admin", "network", "wireless"}, arcombine(template("admin_network/wifi_overview"), cbi("admin_network/wifi")), _("Wireless"), 15)
			page.leaf = true
			page.subindex = true

			if page.inreq then
				local wdev
				local net = require "luci.model.network".init(uci)
				for _, wdev in ipairs(net:get_wifidevs()) do
					local wnet
					for _, wnet in ipairs(wdev:get_wifinets()) do
						entry(
							{"admin", "network", "wireless", wnet:id()},
							alias("admin", "network", "wireless"),
							wdev:name() .. ": " .. wnet:shortname()
						)
					end
				end
			end
		end


		page = entry({"admin", "network", "iface_add"}, cbi("admin_network/iface_add"), nil)
		page.leaf = true

		page = entry({"admin", "network", "iface_delete"}, post("iface_delete"), nil)
		page.leaf = true

		page = entry({"admin", "network", "iface_status"}, call("iface_status"), nil)
		page.leaf = true

		page = entry({"admin", "network", "iface_reconnect"}, post("iface_reconnect"), nil)
		page.leaf = true

		page = entry({"admin", "network", "iface_shutdown"}, post("iface_shutdown"), nil)
		page.leaf = true

		page = entry({"admin", "network", "network"}, arcombine(cbi("admin_network/network"), cbi("admin_network/ifaces")), _("Interfaces"), 10)
		page.leaf   = true
		page.subindex = true

		if page.inreq then
			uci:foreach("network", "interface",
				function (section)
					local ifc = section[".name"]
					if ifc ~= "loopback" then
						entry({"admin", "network", "network", ifc},
						true, ifc:upper())
					end
				end)
		end


		if nixio.fs.access("/etc/config/dhcp") then
			page = node("admin", "network", "dhcp")
			page.target = cbi("admin_network/dhcp")
			page.title  = _("DHCP and DNS")
			page.order  = 30

			page = entry({"admin", "network", "dhcplease_status"}, call("lease_status"), nil)
			page.leaf = true

			page = node("admin", "network", "hosts")
			page.target = cbi("admin_network/hosts")
			page.title  = _("Hostnames")
			page.order  = 40
		end

		page  = node("admin", "network", "routes")
		page.target = cbi("admin_network/routes")
		page.title  = _("Static Routes")
		page.order  = 50

		page = node("admin", "network", "diagnostics")
		page.target = template("admin_network/diagnostics")
		page.title  = _("Diagnostics")
		page.order  = 60

		page = entry({"admin", "network", "diag_ping"}, post("diag_ping"), nil)
		page.leaf = true

		page = entry({"admin", "network", "diag_nslookup"}, post("diag_nslookup"), nil)
		page.leaf = true

		page = entry({"admin", "network", "diag_traceroute"}, post("diag_traceroute"), nil)
		page.leaf = true

		page = entry({"admin", "network", "diag_ping6"}, post("diag_ping6"), nil)
		page.leaf = true

		page = entry({"admin", "network", "diag_traceroute6"}, post("diag_traceroute6"), nil)
		page.leaf = true
--	end
end

function wifi_join()
	local tpl  = require "luci.template"
	local http = require "luci.http"
	local dev  = http.formvalue("device")
	local ssid = http.formvalue("join")

	if dev and ssid then
		local cancel = (http.formvalue("cancel") or http.formvalue("cbi.cancel"))
		if not cancel then
			local cbi = require "luci.cbi"
			local map = luci.cbi.load("admin_network/wifi_add")[1]

			if map:parse() ~= cbi.FORM_DONE then
				tpl.render("header")
				map:render()
				tpl.render("footer")
			end

			return
		end
	end

	tpl.render("admin_network/wifi_join")
end

function wifi_add()
	local dev = luci.http.formvalue("device")
	local ntm = require "luci.model.network".init()

	dev = dev and ntm:get_wifidev(dev)

	if dev then
		local net = dev:add_wifinet({
			mode       = "ap",
			ssid       = "OpenWrt",
			encryption = "none"
		})

		ntm:save("wireless")
		luci.http.redirect(net:adminlink())
	end
end

function wifi_delete(network)
	local ntm = require "luci.model.network".init()
	local wnet = ntm:get_wifinet(network)
	if wnet then
		local dev = wnet:get_device()
		local nets = wnet:get_networks()
		if dev then
			ntm:del_wifinet(network)
			ntm:commit("wireless")
			local _, net
			for _, net in ipairs(nets) do
				if net:is_empty() then
					ntm:del_network(net:name())
					ntm:commit("network")
				end
			end
			luci.sys.call("env -i /bin/ubus call network reload >/dev/null 2>/dev/null")
		end
	end

	luci.http.redirect(luci.dispatcher.build_url("admin/network/wireless"))
end

function iface_status(ifaces)
	local netm = require "luci.model.network".init()
	local rv   = { }

	local iface
	for iface in ifaces:gmatch("[%w%.%-_]+") do
		local net = netm:get_network(iface)
		local device = net and net:get_interface()
		if device then
			-- FIXME:Workaround:Use the MAC address of L2 device if we can't get the mac of L3 device.
			local devmac = device:mac()
			if (devmac == "00:00:00:00:00:00" and net:proto() == "pppoe") then
				devmac = (netm.interface(net:_ubus("device"),net)):mac() or "00:00:00:00:00:00"
			end

			local data = {
				id         = iface,
				proto      = net:proto(),
				uptime     = net:uptime(),
				gwaddr     = net:gwaddr(),
				ipaddrs    = net:ipaddrs(),
				ip6addrs   = net:ip6addrs(),
				dnsaddrs   = net:dnsaddrs(),
				ip6prefix  = net:ip6prefix(),
				name       = device:shortname(),
				type       = device:type(),
				ifname     = device:name(),
				macaddr    = devmac,
				is_up      = device:is_up(),
				rx_bytes   = device:rx_bytes(),
				tx_bytes   = device:tx_bytes(),
				rx_packets = device:rx_packets(),
				tx_packets = device:tx_packets(),

				subdevices = { }
			}

			for _, device in ipairs(net:get_interfaces() or {}) do
				data.subdevices[#data.subdevices+1] = {
					name       = device:shortname(),
					type       = device:type(),
					ifname     = device:name(),
					macaddr    = device:mac(),
					macaddr    = device:mac(),
					is_up      = device:is_up(),
					rx_bytes   = device:rx_bytes(),
					tx_bytes   = device:tx_bytes(),
					rx_packets = device:rx_packets(),
					tx_packets = device:tx_packets(),
				}
			end

			rv[#rv+1] = data
		else
			rv[#rv+1] = {
				id   = iface,
				name = iface,
				type = "ethernet"
			}
		end
	end

	if #rv > 0 then
		luci.http.prepare_content("application/json")
		luci.http.write_json(rv)
		return
	end

	luci.http.status(404, "No such device")
end

function iface_reconnect(iface)
	local netmd = require "luci.model.network".init()
	local net = netmd:get_network(iface)
	if net then
		luci.sys.call("env -i /sbin/ifup %q >/dev/null 2>/dev/null" % iface)
		luci.http.status(200, "Reconnected")
		return
	end

	luci.http.status(404, "No such interface")
end

function iface_shutdown(iface)
	local netmd = require "luci.model.network".init()
	local net = netmd:get_network(iface)
	if net then
		luci.sys.call("env -i /sbin/ifdown %q >/dev/null 2>/dev/null" % iface)
		luci.http.status(200, "Shutdown")
		return
	end

	luci.http.status(404, "No such interface")
end

function iface_delete(iface)
	local netmd = require "luci.model.network".init()
	local net = netmd:del_network(iface)
	if net then
		luci.sys.call("env -i /sbin/ifdown %q >/dev/null 2>/dev/null" % iface)
		luci.http.redirect(luci.dispatcher.build_url("admin/network/network"))
		netmd:commit("network")
		netmd:commit("wireless")
		return
	end

	luci.http.status(404, "No such interface")
end

function wifi_status(devs)
	local s    = require "luci.tools.status"
	local rv   = { }

	local dev
	for dev in devs:gmatch("[%w%.%-]+") do
		rv[#rv+1] = s.wifi_network(dev)
	end

	if #rv > 0 then
		luci.http.prepare_content("application/json")
		luci.http.write_json(rv)
		return
	end

	luci.http.status(404, "No such device")
end

local function wifi_reconnect_shutdown(shutdown, wnet)
	local netmd = require "luci.model.network".init()
	local net = netmd:get_wifinet(wnet)
	local dev = net:get_device()
	if dev and net then
		dev:set("disabled", nil)
		net:set("disabled", shutdown and 1 or nil)
		netmd:commit("wireless")

		luci.sys.call("env -i /bin/ubus call network reload >/dev/null 2>/dev/null")
		luci.http.status(200, shutdown and "Shutdown" or "Reconnected")

		return
	end

	luci.http.status(404, "No such radio")
end

function wifi_reconnect(wnet)
	wifi_reconnect_shutdown(false, wnet)
end

function wifi_shutdown(wnet)
	wifi_reconnect_shutdown(true, wnet)
end

function lease_status()
	local s = require "luci.tools.status"

	luci.http.prepare_content("application/json")
	luci.http.write('[')
	luci.http.write_json(s.dhcp_leases())
	luci.http.write(',')
	luci.http.write_json(s.dhcp6_leases())
	luci.http.write(']')
end

function switch_status(switches)
	local s = require "luci.tools.status"

	luci.http.prepare_content("application/json")
	luci.http.write_json(s.switch_status(switches))
end

function diag_command(cmd, addr)
	if addr and addr:match("^[a-zA-Z0-9%-%.:_]+$") then
		luci.http.prepare_content("text/plain")

		local util = io.popen(cmd % addr)
		if util then
			while true do
				local ln = util:read("*l")
				if not ln then break end
				luci.http.write(ln)
				luci.http.write("\n")
			end

			util:close()
		end

		return
	end

	luci.http.status(500, "Bad address")
end

function diag_ping(addr)
	diag_command("ping -c 5 -W 1 %q 2>&1", addr)
end

function diag_traceroute(addr)
	diag_command("traceroute -q 1 -w 1 -n %q 2>&1", addr)
end

function diag_nslookup(addr)
	diag_command("nslookup %q 2>&1", addr)
end

function diag_ping6(addr)
	diag_command("ping6 -c 5 %q 2>&1", addr)
end

function diag_traceroute6(addr)
	diag_command("traceroute6 -q 1 -w 2 -n %q 2>&1", addr)
end
