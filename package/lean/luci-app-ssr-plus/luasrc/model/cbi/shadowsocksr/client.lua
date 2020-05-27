-- Copyright (C) 2017 yushi studio <ywb94@qq.com> github.com/ywb94
-- Copyright (C) 2018 lean <coolsnowwolf@gmail.com> github.com/coolsnowwolf
-- Licensed to the public under the GNU General Public License v3.

local m, s, sec, o, kcp_enable
local shadowsocksr = "shadowsocksr"
local uci = luci.model.uci.cursor()
m = Map(shadowsocksr, translate("ShadowSocksR Plus+ Settings"))

m:section(SimpleSection).template = "shadowsocksr/status"

local server_table = {}
uci:foreach(shadowsocksr, "servers", function(s)
	if s.alias then
		server_table[s[".name"]] = "[%s]:%s" %{string.upper(s.type), s.alias}
	elseif s.server and s.server_port then
		server_table[s[".name"]] = "[%s]:%s:%s" %{string.upper(s.type), s.server, s.server_port}
	end
end)

local key_table = {}
for key,_ in pairs(server_table) do
	table.insert(key_table,key)
end

table.sort(key_table)

-- [[ Global Setting ]]--
s = m:section(TypedSection, "global")
s.anonymous = true

o = s:option(ListValue, "global_server", translate("Main Server"))
o:value("nil", translate("Disable"))
for _,key in pairs(key_table) do o:value(key,server_table[key]) end
o.default = "nil"
o.rmempty = false

o = s:option(ListValue, "udp_relay_server", translate("Game Mode UDP Server"))
o:value("", translate("Disable"))
o:value("same", translate("Same as Global Server"))
for _,key in pairs(key_table) do o:value(key,server_table[key]) end

o = s:option(ListValue, "netflix_server", translate("Netflix Node"))
o:value("nil", translate("Disable"))
o:value("same", translate("Same as Global Server"))
for _,key in pairs(key_table) do o:value(key,server_table[key]) end
o.default = "nil"
o.rmempty = false

o = s:option(Flag, "netflix_proxy", translate("External Proxy Mode"))
o.rmempty = false
o.description = translate("Forward Netflix Proxy through Main Proxy")
o.default="0"

o = s:option(ListValue, "threads", translate("Multi Threads Option"))
o:value("0", translate("Auto Threads"))
o:value("1", translate("1 Thread"))
o:value("2", translate("2 Threads"))
o:value("4", translate("4 Threads"))
o:value("8", translate("8 Threads"))
o:value("16", translate("16 Threads"))
o:value("32", translate("32 Threads"))
o:value("64", translate("64 Threads"))
o:value("128", translate("128 Threads"))
o.default = "0"
o.rmempty = false

o = s:option(ListValue, "run_mode", translate("Running Mode"))
o:value("gfw", translate("GFW List Mode"))
o:value("router", translate("IP Route Mode"))
o:value("all", translate("Global Mode"))
o:value("oversea", translate("Oversea Mode"))
o.default = gfw

o = s:option(ListValue, "dports", translate("Proxy Ports"))
o:value("1", translate("All Ports"))
o:value("2", translate("Only Common Ports"))
o.default = 1

o = s:option(ListValue, "pdnsd_enable", translate("Resolve Dns Mode"))
o:value("1", translate("Use Pdnsd tcp query and cache"))
o:value("2", translate("Use DNS2SOCKS query and cache"))
o:value("0", translate("Use Local DNS Service listen port 5335"))
o.default = 1

o = s:option(Value, "tunnel_forward", translate("Anti-pollution DNS Server"))
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
o:value("1.1.1.1:53", translate("Cloudflare DNS (1.1.1.1)"))
o:value("114.114.114.114:53", translate("Oversea Mode DNS-1 (114.114.114.114)"))
o:value("114.114.115.115:53", translate("Oversea Mode DNS-2 (114.114.115.115)"))
o:depends("pdnsd_enable", "1")
o:depends("pdnsd_enable", "2")
o.description = translate("Custom DNS Server format as IP:PORT (default: 8.8.4.4:53)")

return m

