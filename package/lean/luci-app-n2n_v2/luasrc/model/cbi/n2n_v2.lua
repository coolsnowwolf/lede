--[[
--N2N VPN(V2) configuration page. Made by 981213
--
]] --
local fs = require "nixio.fs"

function get_mask(v)
    v:value("16", "255.255.0.0(16)")
    v:value("17", "255.255.128.0(17)")
    v:value("18", "255.255.192.0(18)")
    v:value("19", "255.255.224.0(19)")
    v:value("20", "255.255.240.0(20)")
    v:value("21", "255.255.248.0(21)")
    v:value("22", "255.255.252.0(22)")
    v:value("23", "255.255.254.0(23)")
    v:value("24", "255.255.255.0(24)")
    v:value("25", "255.255.255.128(25)")
    v:value("26", "255.255.255.192(26)")
    v:value("27", "255.255.255.224(27)")
    v:value("28", "255.255.255.240(28)")
    v:value("29", "255.255.255.248(29)")
    v:value("30", "255.255.255.252(30)")
end

m = Map("n2n_v2", translate("N2N v2 VPN"), translatef(
            "n2n is a layer-two peer-to-peer virtual private network (VPN) which allows users to exploit features typical of P2P applications at network instead of application level."))

-- Basic config
-- edge
m:section(SimpleSection).template = "n2n_v2/status"

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
ipaddr.datatype = "ip4addr"
ipaddr:depends("mode", "static")

prefix = s:option(ListValue, "prefix", translate("Interface netmask"))
get_mask(prefix)
prefix.optional = false
prefix:depends("mode", "static")

mtu = s:option(Value, "mtu", translate("MTU"))
mtu.datatype = "range(1,1500)"
mtu.optional = false

supernode = s:option(Value, "supernode", translate("Supernode Host"))
supernode.datatype = "host"
supernode.optional = false

port = s:option(Value, "port", translate("Supernode Port"))
port.datatype = "port"
port.optional = false

community = s:option(Value, "community", translate("N2N Community name"))
community.optional = false

s:option(Value, "key", translate("Encryption key"))

route = s:option(Flag, "route", translate("Enable packet forwarding"))
route.rmempty = false

-- supernode
s = m:section(TypedSection, "supernode", translate("N2N Supernode Settings"))
s.anonymous = true
s.addremove = true

switch = s:option(Flag, "enabled", translate("Enable"))
switch.rmempty = false

port = s:option(Value, "port", translate("Port"))
port.datatype = "port"
port.optional = false

-- Static route
s = m:section(TypedSection, "route", translate("N2N routes"),
              translate("Static route for n2n interface"))
s.anonymous = true
s.addremove = true
s.template = "cbi/tblsection"

---- enable
switch = s:option(Flag, "enabled", translate("Enable"))
switch.rmempty = false

---- IP address
o = s:option(Value, "ip", translate("IP"))
o.optional = false
o.datatype = "ip4addr"
o.rmempty = false

---- IP mask
o = s:option(ListValue, "mask", translate("Mask"))
o.optional = false
get_mask(o)
o.default = "24"

---- Gateway
o = s:option(Value, "gw", translate("Gateway"))
o.optional = false
o.datatype = "ip4addr"
o.rmempty = false

---- Description
o = s:option(Value, "desc", translate("Description"))
o.optional = false

return m
