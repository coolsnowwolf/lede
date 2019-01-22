-- Copyright (C) 2014-2018 OpenWrt-dist
-- Copyright (C) 2014-2018 Jian Chang <aa65535@live.com>
-- Licensed to the public under the GNU General Public License v3.

local m, s, o

if luci.sys.call("pidof redsocks2 >/dev/null") == 0 then
	m = Map("redsocks2", translate("RedSocks2"), "%s - %s" %{translate("RedSocks2"), translate("RUNNING")})
else
	m = Map("redsocks2", translate("RedSocks2"), "%s - %s" %{translate("RedSocks2"), translate("NOT RUNNING")})
end

-- General Setting
s = m:section(TypedSection, "redsocks2", translate("General Setting"))
s.anonymous = true

o = s:option(Flag, "enable", translate("Enable"))
o.default   = 1
o.rmempty   = false

o = s:option(Value, "local_port", translate("Local Port"))
o.datatype  = "port"
o.rmempty   = false

o = s:option(ListValue, "proxy_type", translate("Proxy Type"))
o:value("socks4", translate("SOCKS4"))
o:value("socks5", translate("SOCKS5"))
o:value("http-connect", translate("HTTP-CONNECT"))
o:value("http-relay", translate("HTTP-RELAY"))
o:value("direct", translate("DIRECT"))
o.rmempty   = false

o = s:option(Value, "proxy_ip", translate("Proxy IP"))
o.datatype  = "ipaddr"
o.rmempty   = false

o = s:option(Value, "proxy_port", translate("Proxy Port"))
o.datatype  = "port"
o.rmempty   = false

o = s:option(ListValue, "auto_proxy", translate("Auto Proxy"))
o:value("1", translate("Enable"))
o:value("0", translate("Disable"))
o.default   = 1
o.rmempty   = false

o = s:option(Value, "timeout", translate("Wait Timeout"))
o.datatype  = "uinteger"
o.default   = 5
o:depends("auto_proxy", 1)

return m
