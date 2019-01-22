
local wa = require "luci.http"
local fs = require "nixio.fs"
local net = require "luci.model.network".init()
local sys = require "luci.sys"
local ifaces = sys.net:devices()

m = Map("sfe")
m.title	= translate("Turbo ACC Acceleration Settings")
m.description = translate("Opensource Qualcomm Shortcut FE driver (Fast Path)")

m:append(Template("sfe/status"))

s = m:section(TypedSection, "sfe", "")
s.addremove = false
s.anonymous = true


enable = s:option(Flag, "enabled", translate("Enable SFE Fast Path"))
enable.default = 0
enable.rmempty = false
enable.description = translate("Enable Fast Path offloading for connections. (decrease cpu load / increase routing throughput)")

wifi = s:option(Flag, "wifi", translate("Bridge Acceleration"))
wifi.default = 0
wifi.rmempty = false
wifi.description = translate("Enable Bridge Acceleration (may be functional conflict with bridge-mode VPN Server)")
wifi:depends("enabled", 1)

ipv6 = s:option(Flag, "ipv6", translate("IPv6 Acceleration"))
ipv6.default = 0
ipv6.rmempty = false
ipv6.description = translate("Enable IPv6 Acceleration")
ipv6:depends("enabled", 1)

bbrenable = s:option(Flag, "bbr_enabled", translate("Enable BBR"))
bbrenable.default = 0
bbrenable.rmempty = false
bbrenable.description = translate("Open source TCP congestion control algorithm")

nat1enable = s:option(Flag, "nat1_enabled", translate("Enable FullConeNat"))
nat1enable.default = 0
nat1enable.rmempty = false
nat1enable.description = translate("FullConeNat NAT1")

nat1allenable = s:option(Flag, "nat1_all_enabled", translate("Enable ALL FullConeNat"))
nat1allenable.default = 0
nat1allenable.rmempty = false
nat1allenable.description = translate("FullConeNat NAT1")
nat1allenable:depends("nat1_enabled", 1)

o = s:option(Value, "fullconenat_ip", translate("FullConeNat IP"))
o.default = "192.168.1.100"
o.rempty = true
o.description = translate("FullConeNat IP:192.168.1.100,192.168.1.101,192.168.1.102")
o:depends("nat1_enabled", 1)

enable = s:option(Flag, "advanced_setting", translate("Advanced Setting"), translate("Multi line user specified exit"))
enable.default = 0
enable.rmempty = false
enable:depends("nat1_enabled", 1)

o = s:option(Value, "interface", translate("wan Interface name"))
-- sm lifted from luci-app-wol, the original implementation failed to show pppoe-ge00 type interface names
for _, iface in ipairs(ifaces) do
	if not (iface == "lo" or iface:match("^ifb.*")) then
		local nets = net:get_interface(iface)
		nets = nets and nets:get_networks() or {}
		for k, v in pairs(nets) do
			nets[k] = nets[k].sid
		end
		nets = table.concat(nets, ",")
		o:value(iface, ((#nets > 0) and "%s (%s)" % {iface, nets} or iface))
	end
end
o.default = "pppoe-wan"
o.rempty      = true
o:depends("advanced_setting", "1")

dns = s:option(Flag, "dns", translate("DNS Acceleration"))
dns.default = 0
dns.rmempty = false
dns.description = translate("Enable DNS Cache Acceleration and anti ISP DNS pollution")

o = s:option(Value, "dns_server", translate("Upsteam DNS Server"))
o.default = "1.0.0.1,1.1.1.1,1.2.4.8,4.2.2.1,4.2.2.2,8.8.4.4,8.8.8.8,9.9.9.9,114.114.114.114,114.114.115.115,119.28.28.28,119.29.29.29,180.76.76.76,210.2.4.8,208.67.220.220,208.67.222.222,223.5.5.5,223.6.6.6,240c::6644,240c::6666,2400:da00::6666,2001:4860:4860::8844,2001:4860:4860::8888,2606:4700:4700::1001,2606:4700:4700::1111"
o.description = translate("Muitiple DNS server can saperate with ','")
o:depends("dns", 1)

return m
