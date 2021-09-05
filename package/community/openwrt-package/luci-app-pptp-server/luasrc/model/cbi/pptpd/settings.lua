local s = require "luci.sys"
local net = require"luci.model.network".init()
local ifaces = s.net:devices()
local m, s, o
m = Map("luci-app-pptp-server", translate("PPTP VPN Server"))
m.description = translate("Simple, quick and convenient PPTP VPN, universal across the platform")
m.template = "pptpd/index"

s = m:section(TypedSection, "service")
s.anonymous = true

o = s:option(DummyValue, "pptpd_status", translate("Current Condition"))
o.template = "pptpd/status"
o.value = translate("Collecting data...")

o = s:option(Flag, "enabled", translate("Enable VPN Server"))
o.rmempty = false

o = s:option(Value, "localip", translate("Server IP"), translate("VPN Server IP address, it not required."))
o.datatype = "ipaddr"
o.placeholder = translate("192.168.1.2")
o.rmempty = true
o.default = "192.168.1.2"

o = s:option(Value, "remoteip", translate("Client IP"), translate("VPN Client IP address, it not required."))
o.placeholder = translate("192.168.1.10-20")
o.rmempty = true
o.default = "192.168.1.10-20"

--[[
o = s:option(Value, "dns", translate("DNS IP address"), translate("This will be sent to the client, it not required."))
o.placeholder = translate("192.168.1.1")
o.datatype = "ipaddr"
o.rmempty = true
o.default = "192.168.1.1"
]]--

o = s:option(Flag, "mppe", translate("Enable MPPE Encryption"), translate("Allows 128-bit encrypted connection."))
o.rmempty = false
return m
