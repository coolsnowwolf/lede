--[[
--N2N VPN(V2) configuration page. Made by 981213
--
]]--

local fs = require "nixio.fs"

m = Map("n2n_v2", translate("N2N v2 VPN"),
        translatef("n2n is a layer-two peer-to-peer virtual private network (VPN) which allows users to exploit features typical of P2P applications at network instead of application level."))

s = m:section(TypedSection, "edge", translate("N2N Edge Settings"))
s.anonymous = true
s.addremove = true 

switch = s:option(Flag, "enabled", translate("Enable"))
switch.rmempty = false

tunname = s:option(Value, "tunname", translate("TUN desvice name"))
tunname.optional = false

mode = s:option(ListValue, "mode", translate("Interface mode"))
mode:value("dhcp")
mode:value("static")

ipaddr = s:option(Value, "ipaddr", translate("Interface IP address"))
ipaddr.optional = false

netmask = s:option(Value, "netmask", translate("Interface netmask"))
netmask.optional = false

mtu = s:option(Value, "mtu", translate("MTU"))
mtu.datatype = "range(1,1500)"
mtu.optional = false

supernode = s:option(Value, "supernode", translate("Supernode IP address"))
supernode.optional = false

port = s:option(Value, "port", translate("Supernode Port"))
port.datatype = "range(0,65535)"
port.optional = false

community = s:option(Value, "community", translate("N2N Community name"))
community.optional = false

s:option(Value, "key", translate("Encryption key"))

route = s:option(Flag, "route", translate("Enable packet forwarding"))
route.rmempty = false

s2 = m:section(TypedSection, "supernode", translate("N2N Supernode Settings"))
s2.anonymous = true
s2.addremove = true 

switch = s2:option(Flag, "enabled", translate("Enable"))
switch.rmempty = false

port = s2:option(Value, "port", translate("Port"))
port.datatype = "range(0,65535)"
port.optional = false

return m


