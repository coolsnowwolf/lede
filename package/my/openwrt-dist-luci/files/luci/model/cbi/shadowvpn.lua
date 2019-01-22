-- Copyright (C) 2014-2018 OpenWrt-dist
-- Copyright (C) 2014-2018 Jian Chang <aa65535@live.com>
-- Licensed to the public under the GNU General Public License v3.

local m, s, o

if luci.sys.call("pidof shadowvpn >/dev/null") == 0 then
	m = Map("shadowvpn", translate("ShadowVPN"), "%s - %s" %{translate("ShadowVPN"), translate("RUNNING")})
else
	m = Map("shadowvpn", translate("ShadowVPN"), "%s - %s" %{translate("ShadowVPN"), translate("NOT RUNNING")})
end

-- General Setting
s = m:section(TypedSection, "shadowvpn", translate("General Setting"))
s.anonymous   = true

o = s:option(Flag, "enable", translate("Enable"))
o.default     = 1
o.rmempty     = false

o = s:option(Value, "server", translate("Server Address"))
o.datatype    = "host"
o.rmempty     = false

o = s:option(Value, "port", translate("Server Port"))
o.datatype    = "port"
o.rmempty     = false

o = s:option(Value, "user_token", translate("User Token"))
o:value("0000000000000000", translate("Disabled"))
o.default     = "0000000000000000"
o.datatype    = "rangelength(16,16)"
o.rmempty     = false

o = s:option(Value, "password", translate("Password"))
o.password    = true
o.rmempty     = false

o = s:option(Value, "net", translate("Local Subnet"))
o.placeholder = "10.7.0.2/24"
o.default     = "10.7.0.2/24"
o.datatype    = "ip4addr"
o.rmempty     = false

o = s:option(Value, "mtu", translate("Override MTU"))
o.placeholder = 1440
o.default     = 1440
o.datatype    = "range(296,1500)"
o.rmempty     = false

o = s:option(Value, "intf", translate("Interface Name"))
o.placeholder = "tun0"
o.default     = "tun0"
o.rmempty     = false

-- Route Configuration
s = m:section(TypedSection, "shadowvpn", translate("Routing Configuration"))
s.anonymous   = true

o = s:option(ListValue, "route_mode", translate("Routing Mode"))
o:value("0", translate("Global Mode"))
o:value("1", translate("Domestic Routes"))
o:value("2", translate("Foreign Routes"))
o.default     = "0"
o.rmempty     = false

o = s:option(Value, "route_file", translate("Routing File"))
o.datatype    = "file"
o:depends("route_mode", "1")
o:depends("route_mode", "2")

return m
