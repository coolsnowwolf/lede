-- Copyright (C) 2017 yushi studio <ywb94@qq.com> github.com/ywb94
-- Copyright (C) 2018 lean <coolsnowwolf@gmail.com> github.com/coolsnowwolf
-- Licensed to the public under the GNU General Public License v3.

local m, s, sec, o, kcp_enable
local shadowsocksr = "shadowsocksr"
local uci = luci.model.uci.cursor()

local sys = require "luci.sys"

m = Map(shadowsocksr, translate("ShadowSocksR Plus+ Settings"))

m:section(SimpleSection).template  = "shadowsocksr/status"

local server_table = {}
uci:foreach(shadowsocksr, "servers", function(s)
	if s.alias then
		server_table[s[".name"]] = s.alias
	elseif s.server and s.server_port then
		server_table[s[".name"]] = "%s:%s" %{s.server, s.server_port}
	end
end)

-- [[ Global Setting ]]--
s = m:section(TypedSection, "global")
s.anonymous = true

o = s:option(ListValue, "global_server", translate("Main Server"))
o:value("nil", translate("Disable"))
for k, v in pairs(server_table) do o:value(k, v) end
o.default = "nil"
o.rmempty = false

o = s:option(ListValue, "udp_relay_server", translate("UDP Relay Server"))
o:value("", translate("Disable"))
o:value("same", translate("Same as Global Server"))
for k, v in pairs(server_table) do o:value(k, v) end

if nixio.fs.access("/usr/bin/ssr-gfw") then
o = s:option(ListValue, "run_mode", translate("Running Mode"))
o:value("gfw", translate("GFW List Mode"))
o:value("router", translate("IP Route Mode"))
o.default = gfw

o = s:option(ListValue, "pdnsd_enable", translate("Resolve Dns Mode"))
o:depends("run_mode", "gfw")
o:value("1", translate("Use Pdnsd tcp query and cache"))
o:value("0", translate("Use SSR DNS Tunnel"))
o.default = 1

o = s:option(Flag, "tunnel_enable", translate("Enable Tunnel(DNS)"))
o:depends("run_mode", "router")
o.default = 0

o = s:option(Value, "tunnel_port", translate("Tunnel Port"))
o:depends("run_mode", "router")
o.datatype = "port"
o.default = 5300
else
o = s:option(Flag, "tunnel_enable", translate("Enable Tunnel(DNS)"))
o.default = 0

o = s:option(Value, "tunnel_port", translate("Tunnel Port"))
o.datatype = "port"
o.default = 5300
end

o = s:option(ListValue, "tunnel_forward", translate("Anti-pollution DNS Server"))
o:value("8.8.4.4:53", translate("Google Public DNS (8.8.4.4)"))
o:value("8.8.8.8:53", translate("Google Public DNS (8.8.8.8)"))
o:value("208.67.222.222:53", translate("OpenDNS (208.67.222.222)"))
o:value("208.67.220.220:53", translate("OpenDNS (208.67.220.220)"))
o:value("209.244.0.3:53", translate("Level 3 Public DNS (209.244.0.3)"))
o:value("209.244.0.4:53", translate("Level 3 Public DNS (209.244.0.4)"))
o:value("4.2.2.1:53", translate("Level 3 Public DNS (4.2.2.1)"))
o:value("4.2.2.2:53", translate("Level 3 Public DNS (4.2.2.2)"))
o:value("4.2.2.3:53", translate("Level 3 Public DNS (4.2.2.3)"))
o:value("4.2.2.4:53", translate("Level 3 Public DNS (4.2.2.4)"))

return m
