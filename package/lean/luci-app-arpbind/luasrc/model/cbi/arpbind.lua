--[[
 静态ARP绑定 Luci页面 CBI page
 Copyright (C) 2015 GuoGuo <gch981213@gmail.com>
]]--

local sys = require "luci.sys"
local ifaces = sys.net:devices()

m = Map("arpbind", translate("IP/MAC Binding"),
        translatef("ARP is used to convert a network address (e.g. an IPv4 address) to a physical address such as a MAC address.Here you can add some static ARP binding rules."))

s = m:section(TypedSection, "arpbind", translate("Rules"))
s.template = "cbi/tblsection"
s.anonymous = true
s.addremove = true

a = s:option(Value, "ipaddr", translate("IP Address"))
a.datatype = "ipaddr"
a.optional = false

a = s:option(Value, "macaddr", translate("MAC Address"))
a.datatype = "macaddr"
a.optional = false

a = s:option(ListValue, "ifname", translate("Interface"))
for _, iface in ipairs(ifaces) do
	if iface ~= "lo" then 
		a:value(iface) 
	end
end
a.default = "br-lan"
a.rmempty = false

return m


