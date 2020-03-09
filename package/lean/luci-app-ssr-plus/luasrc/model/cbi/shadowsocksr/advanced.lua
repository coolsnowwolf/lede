
m = Map("shadowsocksr")
-- [[ global ]]--
s = m:section(TypedSection, "global", translate("Server failsafe auto swith and custom update settings"))
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

o = s:option(Flag, "chnroute", translate("Enable Custom Chnroute"))
o.rmempty = false

o = s:option(Value, "chnroute_url", translate("Chnroute Update url"))
o.default = "https://ispip.clang.cn/all_cn.txt"
o:depends("chnroute", "1")

o = s:option(Flag, "adblock", translate("Enable adblock"))
o.rmempty = false

o = s:option(Value, "adblock_url", translate("adblock_url"))
o.default = "https://gitee.com/privacy-protection-tools/anti-ad/raw/master/anti-ad-for-dnsmasq.conf"
o:depends("adblock", "1")
o.description = translate("Support AdGuardHome and DNSMASQ format list")

-- [[ SOCKS Proxy ]]--
if nixio.fs.access("/usr/bin/microsocks") then
s = m:section(TypedSection, "socks5_proxy", translate("SOCKS5 Proxy Server Settings"))
s.anonymous = true

o = s:option(Flag, "socks", translate("Enable SOCKS5 Proxy Server"))
o.rmempty = false

o = s:option(Value, "local_port", translate("Local Port"))
o.datatype = "port"
o.default = 10800
o.rmempty = true
o:depends("socks", "1")

o = s:option(Flag, "auth_enable", translate("Enable Authentication"))
o.rmempty = false
o.default = "0"
o:depends("socks", "1")

o = s:option(Value, "username", translate("Username"))
o.default = "username"
o:depends("auth_enable", "1")

o = s:option(Value, "password", translate("Password"))
o.password = true
o.default = "password"
o:depends("auth_enable", "1")

o = s:option(Flag, "wan_enable", translate("Enable WAN Access"))
o.rmempty = true
o.default = "0"
o:depends("auth_enable", "1")

end
return m
