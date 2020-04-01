local shadowsocksr = "shadowsocksr"
local uci = luci.model.uci.cursor()
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

m = Map("shadowsocksr")
-- [[ global ]]--
s = m:section(TypedSection, "global", translate("Server failsafe auto swith and custom update settings"))
s.anonymous = true

-- o = s:option(Flag, "monitor_enable", translate("Enable Process Deamon"))
-- o.rmempty = false
-- o.default = "1"

o = s:option(Flag, "enable_switch", translate("Enable Auto Switch"))
o.rmempty = false
o.default = "1"

o = s:option(Value, "switch_time", translate("Switch check cycly(second)"))
o.datatype = "uinteger"
o:depends("enable_switch", "1")
o.default = 667

o = s:option(Value, "switch_timeout", translate("Check timout(second)"))
o.datatype = "uinteger"
o:depends("enable_switch", "1")
o.default = 5

o = s:option(Value, "switch_try_count", translate("Check Try Count"))
o.datatype = "uinteger"
o:depends("enable_switch", "1")
o.default = 3

o = s:option(Flag, "adblock", translate("Enable adblock"))
o.rmempty = false

o = s:option(Value, "adblock_url", translate("adblock_url"))
o:value("https://gitee.com/privacy-protection-tools/anti-ad/raw/master/anti-ad-for-dnsmasq.conf", translate("anti-AD"))
o.default = "https://gitee.com/privacy-protection-tools/anti-ad/raw/master/anti-ad-for-dnsmasq.conf"
o:depends("adblock", "1")
o.description = translate("Support AdGuardHome and DNSMASQ format list")

o = s:option(Value, "gfwlist_url", translate("gfwlist Update url"))
o:value("https://cdn.jsdelivr.net/gh/Loukky/gfwlist-by-loukky/gfwlist.txt", translate("Loukky/gfwlist-by-loukky"))
o:value("https://cdn.jsdelivr.net/gh/gfwlist/gfwlist/gfwlist.txt", translate("gfwlist/gfwlist"))
o.default = "https://cdn.jsdelivr.net/gh/gfwlist/gfwlist/gfwlist.txt"

o = s:option(Value, "chnroute_url", translate("Chnroute Update url"))
o:value("https://ispip.clang.cn/all_cn.txt", translate("Clang.CN"))
o.default = "https://ispip.clang.cn/all_cn.txt"

o = s:option(Value, "nfip_url", translate("nfip_url"))
o:value("https://raw.githubusercontent.com/QiuSimons/Netflix_IP/master/NF_only.txt", translate("Netflix IP Only"))
o:value("https://raw.githubusercontent.com/QiuSimons/Netflix_IP/master/getflix.txt", translate("Netflix and AWS"))
o.default = "https://raw.githubusercontent.com/QiuSimons/Netflix_IP/master/NF_only.txt"
o.description = translate("Customize Netflix IP Url")

-- [[ SOCKS5 Proxy ]]--
s = m:section(TypedSection, "socks5_proxy", translate("Global SOCKS5 Proxy Server"))
s.anonymous = true

o = s:option(ListValue, "server", translate("Server"))
o:value("nil", translate("Disable"))
o:value("same", translate("Same as Global Server"))
for _,key in pairs(key_table) do o:value(key,server_table[key]) end
o.default = "nil"
o.rmempty = false

o = s:option(Value, "local_port", translate("Local Port"))
o.datatype = "port"
o.default = 1080
o.rmempty = false

return m
