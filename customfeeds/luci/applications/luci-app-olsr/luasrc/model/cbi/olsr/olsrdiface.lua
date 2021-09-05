-- Copyright 2010 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local util = require "luci.util"
local ip = require "luci.ip"

function write_float(self, section, value)
    local n = tonumber(value)
    if n ~= nil then
        return Value.write(self, section, "%.1f" % n)
    end
end

m = Map("olsrd", translate("OLSR Daemon - Interface"),
        translate("The OLSR daemon is an implementation of the Optimized Link State Routing protocol. "..
	"As such it allows mesh routing for any network equipment. "..
	"It runs on any wifi card that supports ad-hoc mode and of course on any ethernet device. "..
	"Visit <a href='http://www.olsr.org'>olsrd.org</a> for help and documentation."))

m.redirect = luci.dispatcher.build_url("admin/services/olsrd")

if not arg[1] or m.uci:get("olsrd", arg[1]) ~= "Interface" then
	luci.http.redirect(m.redirect)
	return
end

i = m:section(NamedSection, arg[1], "Interface", translate("Interface"))
i.anonymous = true
i.addremove = false

i:tab("general", translate("General Settings"))
i:tab("addrs",   translate("IP Addresses"))
i:tab("timing",  translate("Timing and Validity"))

ign = i:taboption("general", Flag, "ignore", translate("Enable"),
	translate("Enable this interface."))
ign.enabled  = "0"
ign.disabled = "1"
ign.rmempty = false
function ign.cfgvalue(self, section)
	return Flag.cfgvalue(self, section) or "0"
end

network = i:taboption("general", Value, "interface", translate("Network"),
	translate("The interface OLSRd should serve."))

network.template = "cbi/network_netlist"
network.widget   = "radio"
network.nocreate = true

mode = i:taboption("general", ListValue, "Mode", translate("Mode"),
	translate("Interface Mode is used to prevent unnecessary packet forwarding on switched ethernet interfaces. "..
	"valid Modes are \"mesh\" and \"ether\". Default is \"mesh\"."))
mode:value("mesh")
mode:value("ether")
mode.optional = true
mode.rmempty = true


weight = i:taboption("general", Value, "Weight", translate("Weight"),
	translate("When multiple links exist between hosts the weight of interface is used to determine the link to use. "..
	"Normally the weight is automatically calculated by olsrd based on the characteristics of the interface, "..
	"but here you can specify a fixed value. Olsrd will choose links with the lowest value.<br />"..
	"<b>Note:</b> Interface weight is used only when LinkQualityLevel is set to 0. "..
	"For any other value of LinkQualityLevel, the interface ETX value is used instead."))
weight.optional = true
weight.datatype = "uinteger"
weight.placeholder = "0"

lqmult = i:taboption("general", DynamicList, "LinkQualityMult", translate("LinkQuality Multiplicator"),
	translate("Multiply routes with the factor given here. Allowed values are between 0.01 and 1.0. "..
	"It is only used when LQ-Level is greater than 0. Examples:<br />"..
	"reduce LQ to 192.168.0.1 by half: 192.168.0.1 0.5<br />"..
	"reduce LQ to all nodes on this interface by 20%: default 0.8"))
lqmult.optional = true
lqmult.rmempty = true
lqmult.cast = "table"
lqmult.placeholder = "default 1.0"

function lqmult.validate(self, value)
	for _, v in pairs(value) do
		if v ~= "" then
			local val = util.split(v, " ")
			local host = val[1]
			local mult = val[2]
			if not host or not mult then
				return nil, translate("LQMult requires two values (IP address or 'default' and multiplicator) seperated by space.")
			end
			if not (host == "default" or ip.IPv4(host) or ip.IPv6(host)) then
				return nil, translate("Can only be a valid IPv4 or IPv6 address or 'default'")
			end
			if not tonumber(mult) or tonumber(mult) > 1 or tonumber(mult) < 0.01 then
				return nil, translate("Invalid Value for LQMult-Value. Must be between 0.01 and 1.0.")
			end
			if not mult:match("[0-1]%.[0-9]+") then
				return nil, translate("Invalid Value for LQMult-Value. You must use a decimal number between 0.01 and 1.0 here.")
			end
		end
	end
	return value
end

ip4b = i:taboption("addrs", Value, "Ip4Broadcast", translate("IPv4 broadcast"),
	translate("IPv4 broadcast address for outgoing OLSR packets. One useful example would be 255.255.255.255. "..
	"Default is \"0.0.0.0\", which triggers the usage of the interface broadcast IP."))
ip4b.optional = true
ip4b.datatype = "ip4addr"
ip4b.placeholder = "0.0.0.0"

ip6m = i:taboption("addrs", Value, "IPv6Multicast", translate("IPv6 multicast"),
	translate("IPv6 multicast address. Default is \"FF02::6D\", the manet-router linklocal multicast."))
ip6m.optional = true
ip6m.datatype = "ip6addr"
ip6m.placeholder = "FF02::6D"

ip4s = i:taboption("addrs", Value, "IPv4Src", translate("IPv4 source"),
	translate("IPv4 src address for outgoing OLSR packages. Default is \"0.0.0.0\", which triggers usage of the interface IP."))
ip4s.optional = true
ip4s.datatype = "ip4addr"
ip4s.placeholder = "0.0.0.0"

ip6s = i:taboption("addrs", Value, "IPv6Src", translate("IPv6 source"),
	translate("IPv6 src prefix. OLSRd will choose one of the interface IPs which matches the prefix of this parameter. "..
	"Default is \"0::/0\", which triggers the usage of a not-linklocal interface IP."))
ip6s.optional = true
ip6s.datatype = "ip6addr"
ip6s.placeholder = "0::/0"

hi = i:taboption("timing", Value, "HelloInterval", translate("Hello interval"))
hi.optional = true
hi.datatype = "ufloat"
hi.placeholder = "5.0"
hi.write = write_float

hv = i:taboption("timing", Value, "HelloValidityTime", translate("Hello validity time"))
hv.optional = true
hv.datatype = "ufloat"
hv.placeholder = "40.0"
hv.write = write_float

ti = i:taboption("timing", Value, "TcInterval", translate("TC interval"))
ti.optional = true
ti.datatype = "ufloat"
ti.placeholder = "2.0"
ti.write = write_float

tv = i:taboption("timing", Value, "TcValidityTime", translate("TC validity time"))
tv.optional = true
tv.datatype = "ufloat"
tv.placeholder = "256.0"
tv.write = write_float

mi = i:taboption("timing", Value, "MidInterval", translate("MID interval"))
mi.optional = true
mi.datatype = "ufloat"
mi.placeholder = "18.0"
mi.write = write_float

mv = i:taboption("timing", Value, "MidValidityTime", translate("MID validity time"))
mv.optional = true
mv.datatype = "ufloat"
mv.placeholder = "324.0"
mv.write = write_float

ai = i:taboption("timing", Value, "HnaInterval", translate("HNA interval"))
ai.optional = true
ai.datatype = "ufloat"
ai.placeholder = "18.0"
ai.write = write_float

av = i:taboption("timing", Value, "HnaValidityTime", translate("HNA validity time"))
av.optional = true
av.datatype = "ufloat"
av.placeholder = "108.0"
av.write = write_float

return m
