--[[
--N2N VPN(V2) configuration page. Made by 981213
--
]]--

local fs = require "nixio.fs"

m = Map("n2n_v2", translate("N2N v2 VPN"),
        translatef("n2n is a layer-two peer-to-peer virtual private network (VPN) which allows users to exploit features typical of P2P applications at network instead of application level."))

m:section(SimpleSection).template  = "n2n_v2/status"

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
prefix:value("1 ", "128.0.0.0(1)")
prefix:value("2 ", "192.0.0.0(2)")
prefix:value("3 ", "224.0.0.0(3)")
prefix:value("4 ", "240.0.0.0(4)")
prefix:value("5 ", "248.0.0.0(5)")
prefix:value("6 ", "252.0.0.0(6)")
prefix:value("7 ", "254.0.0.0(7)")
prefix:value("8 ", "255.0.0.0(8)")
prefix:value("9 ", "255.128.0.0(9)")
prefix:value("10", "255.192.0.0(10)")
prefix:value("11", "255.224.0.0(11)")
prefix:value("12", "255.240.0.0(12)")
prefix:value("13", "255.248.0.0(13)")
prefix:value("14", "255.252.0.0(14)")
prefix:value("15", "255.254.0.0(15)")
prefix:value("16", "255.255.0.0(16)")
prefix:value("17", "255.255.128.0(17)")
prefix:value("18", "255.255.192.0(18)")
prefix:value("19", "255.255.224.0(19)")
prefix:value("20", "255.255.240.0(20)")
prefix:value("21", "255.255.248.0(21)")
prefix:value("22", "255.255.252.0(22)")
prefix:value("23", "255.255.254.0(23)")
prefix:value("24", "255.255.255.0(24)")
prefix:value("25", "255.255.255.128(25)")
prefix:value("26", "255.255.255.192(26)")
prefix:value("27", "255.255.255.224(27)")
prefix:value("28", "255.255.255.240(28)")
prefix:value("29", "255.255.255.248(29)")
prefix:value("30", "255.255.255.252(30)")
prefix:value("31", "255.255.255.254(31)")
prefix:value("32", "255.255.255.255(32)")
prefix.optional = false
prefix:depends("mode", "static")

mtu = s:option(Value, "mtu", translate("MTU"))
mtu.datatype = "range(1,1500)"
mtu.optional = false

supernode = s:option(Value, "supernode", translate("Supernode IP address"))
supernode.optional = false

port = s:option(Value, "port", translate("Supernode Port"))
port.datatype = "port"
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
port.datatype = "port"
port.optional = false

return m


