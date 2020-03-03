
m = Map("shadowsocksr")
-- [[ global ]]--
s = m:section(TypedSection, "global", translate("Server failsafe auto swith settings"))
s.anonymous = true

o = s:option(Flag, "monitor_enable", translate("Enable Process Deamon"))
o.rmempty = false
o.default = "1"

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

-- [[ adblock ]]--
s = m:section(TypedSection, "global", translate("adblock settings"))
s.anonymous = true

o = s:option(Flag, "adblock", translate("Enable adblock"))
o.rmempty = false

o = s:option(Value, "adblock_url", translate("adblock_url"))
o.default = "https://easylist-downloads.adblockplus.org/easylistchina+easylist.txt"

-- [[ chnroute ]]
s = m:section(TypedSection, "global", translate("Chnroute Setting"))
s.anonymous = true

o = s:option(Flag, "chnroute", translate("Enable custom chnroute"))
o.rmempty = false

o = s:option(Value, "chnroute_url", translate("Update url"))
o.default = "https://cdn.jsdelivr.net/gh/17mon/china_ip_list/china_ip_list.txt"

-- [[ SOCKS Proxy ]]--
if nixio.fs.access("/usr/bin/microsocks") then
s = m:section(TypedSection, "socks5_proxy", translate("SOCKS5 Proxy Server Settings"))
s.anonymous = true

o = s:option(Flag, "socks", translate("Enable SOCKS5 Proxy Server"))
o.rmempty = false

o = s:option(Value, "local_port", translate("Local Port"))
o.datatype = "port"
o.default = 1080
o.rmempty = false

o = s:option(Flag, "auth_enable", translate("Enable Authentication"))
o.rmempty = false
o.default = "0"

o = s:option(Value, "username", translate("Username"))
o.rmempty = false
o:depends("auth_enable", "1")

o = s:option(Value, "password", translate("Password"))
o.password = true
o.rmempty = false
o:depends("auth_enable", "1")

end
return m
