-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local wa  = require "luci.tools.webadmin"
local sys = require "luci.sys"
local fs  = require "nixio.fs"
local nx  = require "nixio"

local has_pptp  = fs.access("/usr/sbin/pptp")
local has_pppoe = fs.glob("/usr/lib/pppd/*/rp-pppoe.so")()

local network = luci.model.uci.cursor_state():get_all("network")

local netstat = {}
local ifaces = {}

local k, v
for k, v in ipairs(nx.getifaddrs()) do
	if v.family == "packet" then
		local d = v.data
		d[1] = d.rx_bytes
		d[2] = d.rx_packets
		d[3] = d.rx_errors
		d[4] = d.rx_dropped
		d[5] = 0
		d[6] = 0
		d[7] = 0
		d[8] = d.multicast
		d[9] = d.tx_bytes
		d[10] = d.tx_packets
		d[11] = d.tx_errors
		d[12] = d.tx_dropped
		d[13] = 0
		d[14] = d.collisions
		d[15] = 0
		d[16] = 0
		netstat[v.name] = d
	end
end

for k, v in pairs(network) do
	if v[".type"] == "interface" and k ~= "loopback" then
		table.insert(ifaces, v)
	end
end

m = Map("network", translate("Network"))
s = m:section(Table, ifaces, translate("Status"))
s.parse = function() end

s:option(DummyValue, ".name", translate("Network"))

hwaddr = s:option(DummyValue, "_hwaddr",
 translate("<abbr title=\"Media Access Control\">MAC</abbr>-Address"), translate("Hardware Address"))
function hwaddr.cfgvalue(self, section)
	local ix = self.map:get(section, "ifname") or ""
	local mac = fs.readfile("/sys/class/net/" .. ix .. "/address")

	if not mac then
		mac = luci.util.exec("ifconfig " .. ix)
		mac = mac and mac:match(" ([A-F0-9:]+)%s*\n")
	end

	if mac and #mac > 0 then
		return mac:upper()
	end

	return "?"
end


s:option(DummyValue, "ipaddr", translate("<abbr title=\"Internet Protocol Version 4\">IPv4</abbr>-Address"))

s:option(DummyValue, "netmask", translate("<abbr title=\"Internet Protocol Version 4\">IPv4</abbr>-Netmask"))


txrx = s:option(DummyValue, "_txrx",
 translate("Traffic"), translate("transmitted / received"))

function txrx.cfgvalue(self, section)
	local ix = self.map:get(section, "ifname")

	local rx = netstat and netstat[ix] and netstat[ix][1]
	rx = rx and wa.byte_format(tonumber(rx)) or "-"

	local tx = netstat and netstat[ix] and netstat[ix][9]
	tx = tx and wa.byte_format(tonumber(tx)) or "-"

	return string.format("%s / %s", tx, rx)
end

errors = s:option(DummyValue, "_err",
 translate("Errors"), translate("TX / RX"))

function errors.cfgvalue(self, section)
	local ix = self.map:get(section, "ifname")

	local rx = netstat and netstat[ix] and netstat[ix][3]
	local tx = netstat and netstat[ix] and netstat[ix][11]

	rx = rx and tostring(rx) or "-"
	tx = tx and tostring(tx) or "-"

	return string.format("%s / %s", tx, rx)
end



s = m:section(NamedSection, "lan", "interface", translate("Local Network"))
s.addremove = false
s:option(Value, "ipaddr", translate("<abbr title=\"Internet Protocol Version 4\">IPv4</abbr>-Address"))

nm = s:option(Value, "netmask", translate("<abbr title=\"Internet Protocol Version 4\">IPv4</abbr>-Netmask"))
nm:value("255.255.255.0")
nm:value("255.255.0.0")
nm:value("255.0.0.0")

gw = s:option(Value, "gateway", translate("<abbr title=\"Internet Protocol Version 4\">IPv4</abbr>-Gateway") .. translate(" (optional)"))
gw.rmempty = true
dns = s:option(Value, "dns", translate("<abbr title=\"Domain Name System\">DNS</abbr>-Server") .. translate(" (optional)"))
dns.rmempty = true


s = m:section(NamedSection, "wan", "interface", translate("Internet Connection"))
s.addremove = false
p = s:option(ListValue, "proto", translate("Protocol"))
p.override_values = true
p:value("none", "disabled")
p:value("static", translate("manual"))
p:value("dhcp", translate("automatic"))
if has_pppoe then p:value("pppoe", "PPPoE") end
if has_pptp  then p:value("pptp",  "PPTP")  end

function p.write(self, section, value)
	-- Always set defaultroute to PPP and use remote dns
	-- Overwrite a bad variable behaviour in OpenWrt
	if value == "pptp" or value == "pppoe" then
		self.map:set(section, "peerdns", "1")
		self.map:set(section, "defaultroute", "1")
	end
	return ListValue.write(self, section, value)
end

if not ( has_pppoe and has_pptp ) then
	p.description = translate("You need to install \"ppp-mod-pppoe\" for PPPoE or \"pptp\" for PPtP support")
end


ip = s:option(Value, "ipaddr", translate("<abbr title=\"Internet Protocol Version 4\">IPv4</abbr>-Address"))
ip:depends("proto", "static")

nm = s:option(Value, "netmask", translate("<abbr title=\"Internet Protocol Version 4\">IPv4</abbr>-Netmask"))
nm:depends("proto", "static")

gw = s:option(Value, "gateway", translate("<abbr title=\"Internet Protocol Version 4\">IPv4</abbr>-Gateway"))
gw:depends("proto", "static")
gw.rmempty = true

dns = s:option(Value, "dns", translate("<abbr title=\"Domain Name System\">DNS</abbr>-Server"))
dns:depends("proto", "static")
dns.rmempty = true

usr = s:option(Value, "username", translate("Username"))
usr:depends("proto", "pppoe")
usr:depends("proto", "pptp")

pwd = s:option(Value, "password", translate("Password"))
pwd.password = true
pwd:depends("proto", "pppoe")
pwd:depends("proto", "pptp")


-- Allow user to set MSS correction here if the UCI firewall is installed
-- This cures some cancer for providers with pre-war routers
if fs.access("/etc/config/firewall") then
	mssfix = s:option(Flag, "_mssfix",
		translate("Clamp Segment Size"), translate("Fixes problems with unreachable websites, submitting forms or other unexpected behaviour for some ISPs."))
	mssfix.rmempty = false

	function mssfix.cfgvalue(self)
		local value
		m.uci:foreach("firewall", "forwarding", function(s)
			if s.src == "lan" and s.dest == "wan" then
				value = s.mtu_fix
			end
		end)
		return value
	end

	function mssfix.write(self, section, value)
		m.uci:foreach("firewall", "forwarding", function(s)
			if s.src == "lan" and s.dest == "wan" then
				m.uci:set("firewall", s[".name"], "mtu_fix", value)
				m:chain("firewall")
			end
		end)
	end
end

kea = s:option(Flag, "keepalive", translate("automatically reconnect"))
kea:depends("proto", "pppoe")
kea:depends("proto", "pptp")
kea.rmempty = true
kea.enabled = "10"


cod = s:option(Value, "demand", translate("disconnect when idle for"), "s")
cod:depends("proto", "pppoe")
cod:depends("proto", "pptp")
cod.rmempty = true

srv = s:option(Value, "server", translate("<abbr title=\"Point-to-Point Tunneling Protocol\">PPTP</abbr>-Server"))
srv:depends("proto", "pptp")
srv.rmempty = true



return m
