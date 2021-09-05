
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local fs = require "luci.openclash"
local uci = require "luci.model.uci".cursor()
local json = require "luci.jsonc"

font_green = [[<font color="green">]]
font_red = [[<font color="red">]]
font_off = [[</font>]]
bold_on  = [[<strong>]]
bold_off = [[</strong>]]

local op_mode = string.sub(luci.sys.exec('uci get openclash.config.operation_mode 2>/dev/null'),0,-2)
if not op_mode then op_mode = "redir-host" end
local lan_ip=SYS.exec("uci -q get network.lan.ipaddr |awk -F '/' '{print $1}' 2>/dev/null |tr -d '\n' || ip addr show 2>/dev/null | grep -w 'inet' | grep 'global' | grep 'brd' | grep -Eo 'inet [0-9\.]+' | awk '{print $2}' | head -n 1 | tr -d '\n'")

m = Map("openclash", translate("Global Settings(Will Modify The Config File Or Subscribe According To The Settings On This Page)"))
m.pageaction = false
m.description=translate("To restore the default configuration, try accessing:").." <a href='javascript:void(0)' onclick='javascript:restore_config(this)'>http://"..lan_ip.."/cgi-bin/luci/admin/services/openclash/restore</a>"

s = m:section(TypedSection, "openclash")
s.anonymous = true

s:tab("op_mode", translate("Operation Mode"))
s:tab("settings", translate("General Settings"))
s:tab("dns", translate("DNS Setting"))
s:tab("lan_ac", translate("Access Control"))
if op_mode == "fake-ip" then
s:tab("rules", translate("Rules Setting(Access Control)"))
else
s:tab("rules", translate("Rules Setting"))
end
s:tab("dashboard", translate("Dashboard Settings"))
s:tab("rules_update", translate("Rules Update"))
s:tab("geo_update", translate("GEOIP Update"))
s:tab("chnr_update", translate("Chnroute Update"))
s:tab("auto_restart", translate("Auto Restart"))
s:tab("version_update", translate("Version Update"))
s:tab("debug", translate("Debug Logs"))
s:tab("dlercloud", translate("Dler Cloud"))

o = s:taboption("op_mode", ListValue, "en_mode", font_red..bold_on..translate("Select Mode")..bold_off..font_off)
o.description = translate("Select Mode For OpenClash Work, Try Flush DNS Cache If Network Error")
if op_mode == "redir-host" then
o:value("redir-host", translate("redir-host"))
o:value("redir-host-tun", translate("redir-host(tun mode)"))
o:value("redir-host-vpn", translate("redir-host-vpn(game mode)"))
o:value("redir-host-mix", translate("redir-host-mix(tun mix mode)"))
o.default = "redir-host"
else
o:value("fake-ip", translate("fake-ip"))
o:value("fake-ip-tun", translate("fake-ip(tun mode)"))
o:value("fake-ip-vpn", translate("fake-ip-vpn(game mode)"))
o:value("fake-ip-mix", translate("fake-ip-mix(tun mix mode)"))
o.default = "fake-ip"
end

o = s:taboption("op_mode", Flag, "enable_udp_proxy", font_red..bold_on..translate("Proxy UDP Traffics")..bold_off..font_off)
o.description = translate("The Servers Must Support UDP forwarding")..", "..font_red..bold_on..translate("If Docker is Installed, UDP May Not Forward Normally")..bold_off..font_off
o:depends("en_mode", "redir-host")
o:depends("en_mode", "fake-ip")
o.default=1

o = s:taboption("op_mode", ListValue, "stack_type", translate("Select Stack Type"))
o.description = translate("Select Stack Type For TUN Mode, According To The Running Speed on Your Machine")
o:depends("en_mode", "redir-host-tun")
o:depends("en_mode", "fake-ip-tun")
o:depends("en_mode", "redir-host-mix")
o:depends("en_mode", "fake-ip-mix")
o:value("system", translate("System　"))
o:value("gvisor", translate("Gvisor"))
o.default = "system"

o = s:taboption("op_mode", ListValue, "proxy_mode", font_red..bold_on..translate("Proxy Mode")..bold_off..font_off)
o.description = translate("Select Proxy Mode, Use Script Mode Could Prevent Proxy BT traffics If Rules Support, eg.lhie1's")
o:value("rule", translate("Rule Proxy Mode"))
o:value("global", translate("Global Proxy Mode"))
o:value("direct", translate("Direct Proxy Mode"))
o:value("script", translate("Script Proxy Mode (Tun Core Only)"))
o.default = "rule"

o = s:taboption("op_mode", Flag, "ipv6_enable", font_red..bold_on..translate("Proxy IPv6 Traffic")..bold_off..font_off)
o.description = font_red..bold_on..translate("Disable IPv6 DHCP To Avoid Abnormal Connection If You Do Not Use")..bold_off..font_off
o.default=0

o = s:taboption("op_mode", Flag, "china_ip6_route", translate("China IPv6 Route"))
o.description = translate("Bypass The China Network Flows, Improve Performance")
o.default=0
o:depends("ipv6_enable", 1)

o = s:taboption("op_mode", Flag, "disable_udp_quic", font_red..bold_on..translate("Disable QUIC")..bold_off..font_off)
o.description = translate("Prevent YouTube and Others To Use QUIC Transmission")..", "..font_red..bold_on..translate("REJECT UDP Traffic On Port 443")..bold_off..font_off
o.default=1

o = s:taboption("op_mode", Flag, "enable_rule_proxy", font_red..bold_on..translate("Rule Match Proxy Mode")..bold_off..font_off)
o.description = translate("Only Proxy Rules Match, Prevent BT/P2P Passing")
o.default=0

o = s:taboption("op_mode", Flag, "common_ports", font_red..bold_on..translate("Common Ports Proxy Mode")..bold_off..font_off)
o.description = translate("Only Common Ports, Prevent BT/P2P Passing")
o.default=0
o:depends("en_mode", "redir-host")
o:depends("en_mode", "redir-host-tun")
o:depends("en_mode", "redir-host-vpn")
o:depends("en_mode", "redir-host-mix")

o = s:taboption("op_mode", Flag, "china_ip_route", translate("China IP Route"))
o.description = translate("Bypass The China Network Flows, Improve Performance")
o.default=0
o:depends("en_mode", "redir-host")
o:depends("en_mode", "redir-host-tun")
o:depends("en_mode", "redir-host-vpn")
o:depends("en_mode", "redir-host-mix")

o = s:taboption("op_mode", Flag, "small_flash_memory", translate("Small Flash Memory"))
o.description = translate("Move Core And GEOIP Data File To /tmp/etc/openclash For Small Flash Memory Device")
o.default=0

---- Operation Mode
switch_mode = s:taboption("op_mode", DummyValue, "", nil)
switch_mode.template = "openclash/switch_mode"

---- General Settings
o = s:taboption("settings", ListValue, "interface_name", font_red..bold_on..translate("Bind Network Interface")..bold_off..font_off)
local de_int = SYS.exec("ip route |grep 'default' |awk '{print $5}' 2>/dev/null")
o.description = translate("Default Interface Name:").." "..font_green..bold_on..de_int..bold_off..font_off..translate(",Try Enable If Network Loopback")
local interfaces = SYS.exec("ls -l /sys/class/net/ 2>/dev/null |awk '{print $9}' 2>/dev/null")
for interface in string.gmatch(interfaces, "%S+") do
   o:value(interface)
end
o:value("0", translate("Disable"))
o.default=0

o = s:taboption("settings", Value, "tolerance", font_red..bold_on..translate("Url-Test Group Tolerance (ms)")..bold_off..font_off)
o.description = translate("Switch To The New Proxy When The Delay Difference Between Old and The Fastest Currently is Greater Than This Value")
o:value("0", translate("Disable"))
o:value("100")
o:value("150")
o.datatype = "uinteger"
o.default = "0"

o = s:taboption("settings", ListValue, "log_level", translate("Log Level"))
o.description = translate("Select Core's Log Level")
o:value("info", translate("Info Mode"))
o:value("warning", translate("Warning Mode"))
o:value("error", translate("Error Mode"))
o:value("debug", translate("Debug Mode"))
o:value("silent", translate("Silent Mode"))
o.default = "silent"

o = s:taboption("settings", Value, "log_size", translate("Log Size (KB)"))
o.description = translate("Set Log File Size (KB)")
o.default=1024

o = s:taboption("settings", Flag, "intranet_allowed", translate("Only intranet allowed"))
o.description = translate("When Enabled, The Control Panel And The Connection Broker Port Will Not Be Accessible From The Public Network")
o.default=0

o = s:taboption("settings", Value, "dns_port")
o.title = translate("DNS Port")
o.default = 7874
o.datatype = "port"
o.rmempty = false
o.description = translate("Please Make Sure Ports Available")

o = s:taboption("settings", Value, "proxy_port")
o.title = translate("Redir Port")
o.default = 7892
o.datatype = "port"
o.rmempty = false
o.description = translate("Please Make Sure Ports Available")

o = s:taboption("settings", Value, "tproxy_port")
o.title = translate("TProxy Port")
o.default = 7895
o.datatype = "port"
o.rmempty = false
o.description = translate("Please Make Sure Ports Available")

o = s:taboption("settings", Value, "http_port")
o.title = translate("HTTP(S) Port")
o.default = 7890
o.datatype = "port"
o.rmempty = false
o.description = translate("Please Make Sure Ports Available")

o = s:taboption("settings", Value, "socks_port")
o.title = translate("SOCKS5 Port")
o.default = 7891
o.datatype = "port"
o.rmempty = false
o.description = translate("Please Make Sure Ports Available")

o = s:taboption("settings", Value, "mixed_port")
o.title = translate("Mixed Port")
o.default = 7893
o.datatype = "port"
o.rmempty = false
o.description = translate("Please Make Sure Ports Available")

---- DNS Settings
o = s:taboption("dns", Flag, "enable_redirect_dns", font_red..bold_on..translate("Redirect Local DNS Setting")..bold_off..font_off)
o.description = translate("Set Local DNS Redirect")
o.default=1

o = s:taboption("dns", Flag, "enable_custom_dns", font_red..bold_on..translate("Custom DNS Setting")..bold_off..font_off)
o.description = font_red..bold_on..translate("Set OpenClash Upstream DNS Resolve Server")..bold_off..font_off
o.default=0

o = s:taboption("dns", Flag, "append_wan_dns", font_red..bold_on..translate("Append Upstream DNS")..bold_off..font_off)
o.description = font_red..bold_on..translate("Append The Upstream Assigned DNS And Gateway IP To The Nameserver")..bold_off..font_off
o.default=1

o = s:taboption("dns", Flag, "ipv6_dns", translate("IPv6 DNS Resolve"))
o.description = font_red..bold_on..translate("Enable Clash to Resolve ipv6 DNS Requests")..bold_off..font_off
o.default=0

o = s:taboption("dns", Flag, "disable_masq_cache", translate("Disable Dnsmasq's DNS Cache"))
o.description = translate("Recommended Enabled For Avoiding Some Connection Errors")..font_red..bold_on..translate("(Maybe Incompatible For Your Firmware)")..bold_off..font_off
o.default=0

o = s:taboption("dns", Flag, "custom_fallback_filter", translate("Custom Fallback-Filter"))
o.description = translate("Take Effect If Fallback DNS Setted, Prevent DNS Pollution")
o.default=0

custom_fallback_filter = s:taboption("dns", Value, "custom_fallback_fil")
custom_fallback_filter.template = "cbi/tvalue"
custom_fallback_filter.rows = 20
custom_fallback_filter.wrap = "off"
custom_fallback_filter:depends("custom_fallback_filter", "1")

function custom_fallback_filter.cfgvalue(self, section)
	return NXFS.readfile("/etc/openclash/custom/openclash_custom_fallback_filter.yaml") or ""
end
function custom_fallback_filter.write(self, section, value)
	if value then
		value = value:gsub("\r\n?", "\n")
		local old_value = NXFS.readfile("/etc/openclash/custom/openclash_custom_fallback_filter.yaml")
	  if value ~= old_value then
			NXFS.writefile("/etc/openclash/custom/openclash_custom_fallback_filter.yaml", value)
		end
	end
end

o = s:taboption("dns", Flag, "dns_advanced_setting", translate("Advanced Setting"))
o.description = translate("DNS Advanced Settings")..font_red..bold_on..translate("(Please Don't Modify it at Will)")..bold_off..font_off
o.default=0

if op_mode == "fake-ip" then
o = s:taboption("dns", Button, translate("Fake-IP-Filter List Update")) 
o.title = translate("Fake-IP-Filter List Update")
o:depends("dns_advanced_setting", "1")
o.inputtitle = translate("Check And Update")
o.inputstyle = "reload"
o.write = function()
  m.uci:set("openclash", "config", "enable", 1)
  m.uci:commit("openclash")
  SYS.call("rm -rf /tmp/openclash_fake_filter.list >/dev/null 2>&1 && /etc/init.d/openclash restart >/dev/null 2>&1 &")
  HTTP.redirect(DISP.build_url("admin", "services", "openclash"))
end

custom_fake_black = s:taboption("dns", Value, "custom_fake_filter")
custom_fake_black.template = "cbi/tvalue"
custom_fake_black.description = translate("Domain Names In The List Do Not Return Fake-IP, One rule per line")
custom_fake_black.rows = 20
custom_fake_black.wrap = "off"
custom_fake_black:depends("dns_advanced_setting", "1")

function custom_fake_black.cfgvalue(self, section)
	return NXFS.readfile("/etc/openclash/custom/openclash_custom_fake_filter.list") or ""
end
function custom_fake_black.write(self, section, value)
	if value then
		value = value:gsub("\r\n?", "\n")
		local old_value = NXFS.readfile("/etc/openclash/custom/openclash_custom_fake_filter.list")
	  if value ~= old_value then
			NXFS.writefile("/etc/openclash/custom/openclash_custom_fake_filter.list", value)
		end
	end
end
end

o = s:taboption("dns", Value, "custom_domain_dns_server", translate("Specify DNS Server"))
o.description = translate("Specify DNS Server For List and Server Nodes With Fake-IP Mode, Only One IP Server Address Support")
o.default="114.114.114.114"
o.placeholder = translate("114.114.114.114 or 127.0.0.1#5300")
o:depends("dns_advanced_setting", "1")

custom_domain_dns = s:taboption("dns", Value, "custom_domain_dns")
custom_domain_dns.template = "cbi/tvalue"
custom_domain_dns.description = translate("Domain Names In The List Use The Custom DNS Server, One rule per line")
custom_domain_dns.rows = 20
custom_domain_dns.wrap = "off"
custom_domain_dns:depends("dns_advanced_setting", "1")

function custom_domain_dns.cfgvalue(self, section)
	return NXFS.readfile("/etc/openclash/custom/openclash_custom_domain_dns.list") or ""
end
function custom_domain_dns.write(self, section, value)
	if value then
		value = value:gsub("\r\n?", "\n")
		local old_value = NXFS.readfile("/etc/openclash/custom/openclash_custom_domain_dns.list")
	  if value ~= old_value then
			NXFS.writefile("/etc/openclash/custom/openclash_custom_domain_dns.list", value)
		end
	end
end

custom_domain_dns_policy = s:taboption("dns", Value, "custom_domain_dns_core")
custom_domain_dns_policy.template = "cbi/tvalue"
custom_domain_dns_policy.description = translate("Domain Names In The List Use The Custom DNS Server, But Still Return Fake-IP Results, One rule per line")
custom_domain_dns_policy.rows = 20
custom_domain_dns_policy.wrap = "off"
custom_domain_dns_policy:depends("dns_advanced_setting", "1")

function custom_domain_dns_policy.cfgvalue(self, section)
	return NXFS.readfile("/etc/openclash/custom/openclash_custom_domain_dns_policy.list") or ""
end
function custom_domain_dns_policy.write(self, section, value)
	if value then
		value = value:gsub("\r\n?", "\n")
		local old_value = NXFS.readfile("/etc/openclash/custom/openclash_custom_domain_dns_policy.list")
	  if value ~= old_value then
			NXFS.writefile("/etc/openclash/custom/openclash_custom_domain_dns_policy.list", value)
		end
	end
end

---- Access Control
if op_mode == "redir-host" then
o = s:taboption("lan_ac", ListValue, "lan_ac_mode", translate("LAN Access Control Mode"))
o:value("0", translate("Black List Mode"))
o:value("1", translate("White List Mode"))
o.default=0

ip_b = s:taboption("lan_ac", DynamicList, "lan_ac_black_ips", translate("LAN Bypassed Host List"))
ip_b:depends("lan_ac_mode", "0")
ip_b.datatype = "ipaddr"

mac_b = s:taboption("lan_ac", DynamicList, "lan_ac_black_macs", translate("LAN Bypassed Mac List"))
mac_b.datatype = "list(macaddr)"
mac_b.rmempty  = true
mac_b:depends("lan_ac_mode", "0")

ip_w = s:taboption("lan_ac", DynamicList, "lan_ac_white_ips", translate("LAN Proxied Host List"))
ip_w:depends("lan_ac_mode", "1")
ip_w.datatype = "ipaddr"

mac_w = s:taboption("lan_ac", DynamicList, "lan_ac_white_macs", translate("LAN Proxied Mac List"))
mac_w.datatype = "list(macaddr)"
mac_w.rmempty  = true
mac_w:depends("lan_ac_mode", "1")

luci.ip.neighbors({ family = 4 }, function(n)
	if n.mac and n.dest then
		ip_b:value(n.dest:string())
		ip_w:value(n.dest:string())
		mac_b:value(n.mac, "%s (%s)" %{ n.mac, n.dest:string() })
		mac_w:value(n.mac, "%s (%s)" %{ n.mac, n.dest:string() })
	end
end)

luci.ip.neighbors({ family = 6 }, function(n)
	if n.mac and n.dest then
		ip_b:value(n.dest:string())
		ip_w:value(n.dest:string())
		mac_b:value(n.mac, "%s (%s)" %{ n.mac, n.dest:string() })
		mac_w:value(n.mac, "%s (%s)" %{ n.mac, n.dest:string() })
	end
end)
end

o = s:taboption("lan_ac", DynamicList, "wan_ac_black_ips", translate("WAN Bypassed Host List"))
o.datatype = "ipaddr"
o.description = translate("In The Fake-IP Mode, Only Pure IP Requests Are Supported")

---- Rules Settings
o = s:taboption("rules", Flag, "rule_source", translate("Enable Other Rules"))
o.description = translate("Use Other Rules")
o.default=0

if op_mode == "fake-ip" then
o = s:taboption("rules", Flag, "enable_custom_clash_rules", font_red..bold_on..translate("Custom Clash Rules(Access Control)")..bold_off..font_off)
else
o = s:taboption("rules", Flag, "enable_custom_clash_rules", font_red..bold_on..translate("Custom Clash Rules")..bold_off..font_off)
end
o.description = translate("Use Custom Rules")
o.default=0

custom_rules = s:taboption("rules", Value, "custom_rules")
custom_rules:depends("enable_custom_clash_rules", 1)
custom_rules.template = "cbi/tvalue"
custom_rules.description = translate("Custom Priority Rules Here, For More Go:").." "..'<a href="https://lancellc.gitbook.io/clash/clash-config-file/rules">https://lancellc.gitbook.io/clash/clash-config-file/rules</a>'.." ,"..translate("IP To CIDR:").." "..'<a href="http://ip2cidr.com">http://ip2cidr.com</a>'
custom_rules.rows = 20
custom_rules.wrap = "off"

function custom_rules.cfgvalue(self, section)
	return NXFS.readfile("/etc/openclash/custom/openclash_custom_rules.list") or ""
end
function custom_rules.write(self, section, value)
	if value then
		value = value:gsub("\r\n?", "\n")
		local old_value = NXFS.readfile("/etc/openclash/custom/openclash_custom_rules.list")
	  if value ~= old_value then
			NXFS.writefile("/etc/openclash/custom/openclash_custom_rules.list", value)
		end
	end
end

custom_rules_2 = s:taboption("rules", Value, "custom_rules_2")
custom_rules_2:depends("enable_custom_clash_rules", 1)
custom_rules_2.template = "cbi/tvalue"
custom_rules_2.description = translate("Custom Extended Rules Here, For More Go:").." "..'<a href="https://lancellc.gitbook.io/clash/clash-config-file/rules">https://lancellc.gitbook.io/clash/clash-config-file/rules</a>'.." ,"..translate("IP To CIDR:").." "..'<a href="http://ip2cidr.com">http://ip2cidr.com</a>'
custom_rules_2.rows = 20
custom_rules_2.wrap = "off"

function custom_rules_2.cfgvalue(self, section)
	return NXFS.readfile("/etc/openclash/custom/openclash_custom_rules_2.list") or ""
end
function custom_rules_2.write(self, section, value)
	if value then
		value = value:gsub("\r\n?", "\n")
		local old_value = NXFS.readfile("/etc/openclash/custom/openclash_custom_rules_2.list")
	  if value ~= old_value then
			NXFS.writefile("/etc/openclash/custom/openclash_custom_rules_2.list", value)
		end
	end
end

---- update Settings
o = s:taboption("rules_update", Flag, "other_rule_auto_update", translate("Auto Update"))
o.description = font_red..bold_on..translate("Auto Update Other Rules")..bold_off..font_off
o.default=0

o = s:taboption("rules_update", ListValue, "other_rule_update_week_time", translate("Update Time (Every Week)"))
o:value("*", translate("Every Day"))
o:value("1", translate("Every Monday"))
o:value("2", translate("Every Tuesday"))
o:value("3", translate("Every Wednesday"))
o:value("4", translate("Every Thursday"))
o:value("5", translate("Every Friday"))
o:value("6", translate("Every Saturday"))
o:value("0", translate("Every Sunday"))
o.default=1

o = s:taboption("rules_update", ListValue, "other_rule_update_day_time", translate("Update time (every day)"))
for t = 0,23 do
o:value(t, t..":00")
end
o.default=0

o = s:taboption("rules_update", Button, translate("Other Rules Update")) 
o.title = translate("Update Other Rules")
o.inputtitle = translate("Check And Update")
o.description = translate("Other Rules Update(Only in Use)")
o.inputstyle = "reload"
o.write = function()
  m.uci:set("openclash", "config", "enable", 1)
  m.uci:commit("openclash")
  SYS.call("/usr/share/openclash/openclash_rule.sh >/dev/null 2>&1 &")
  HTTP.redirect(DISP.build_url("admin", "services", "openclash"))
end

o = s:taboption("geo_update", Flag, "geo_auto_update", translate("Auto Update"))
o.description = translate("Auto Update GEOIP Database")
o.default=0

o = s:taboption("geo_update", ListValue, "geo_update_week_time", translate("Update Time (Every Week)"))
o:value("*", translate("Every Day"))
o:value("1", translate("Every Monday"))
o:value("2", translate("Every Tuesday"))
o:value("3", translate("Every Wednesday"))
o:value("4", translate("Every Thursday"))
o:value("5", translate("Every Friday"))
o:value("6", translate("Every Saturday"))
o:value("0", translate("Every Sunday"))
o.default=1

o = s:taboption("geo_update", ListValue, "geo_update_day_time", translate("Update time (every day)"))
for t = 0,23 do
o:value(t, t..":00")
end
o.default=0

o = s:taboption("geo_update", Value, "geo_custom_url")
o.title = translate("Custom GEOIP URL")
o.rmempty = false
o.description = translate("Custom GEOIP Data URL, Click Button Below To Refresh After Edit")
o:value("https://cdn.jsdelivr.net/gh/alecthw/mmdb_china_ip_list@release/lite/Country.mmdb", translate("Alecthw-lite-Version")..translate("(Default mmdb)"))
o:value("https://cdn.jsdelivr.net/gh/alecthw/mmdb_china_ip_list@release/Country.mmdb", translate("Alecthw-Version")..translate("(All Info mmdb)"))
o:value("https://cdn.jsdelivr.net/gh/Hackl0us/GeoIP2-CN@release/Country.mmdb", translate("Hackl0us-Version")..translate("(Only CN)"))
o:value("https://static.clash.to/GeoIP2/GeoIP2-Country.mmdb", translate("Static.clash.to"))
o:value("https://geolite.clash.dev/Country.mmdb", translate("Geolite.clash.dev"))
o.default = "http://www.ideame.top/mmdb/Country.mmdb"

o = s:taboption("geo_update", Button, translate("GEOIP Update")) 
o.title = translate("Update GEOIP Database")
o.inputtitle = translate("Check And Update")
o.inputstyle = "reload"
o.write = function()
  m.uci:set("openclash", "config", "enable", 1)
  m.uci:commit("openclash")
  SYS.call("/usr/share/openclash/openclash_ipdb.sh >/dev/null 2>&1 &")
  HTTP.redirect(DISP.build_url("admin", "services", "openclash"))
end

o = s:taboption("chnr_update", Flag, "chnr_auto_update", translate("Auto Update"))
o.description = translate("Auto Update Chnroute Lists")
o.default=0

o = s:taboption("chnr_update", ListValue, "chnr_update_week_time", translate("Update Time (Every Week)"))
o:value("*", translate("Every Day"))
o:value("1", translate("Every Monday"))
o:value("2", translate("Every Tuesday"))
o:value("3", translate("Every Wednesday"))
o:value("4", translate("Every Thursday"))
o:value("5", translate("Every Friday"))
o:value("6", translate("Every Saturday"))
o:value("0", translate("Every Sunday"))
o.default=1

o = s:taboption("chnr_update", ListValue, "chnr_update_day_time", translate("Update time (every day)"))
for t = 0,23 do
o:value(t, t..":00")
end
o.default=0

o = s:taboption("chnr_update", Value, "chnr_custom_url")
o.title = translate("Custom Chnroute Lists URL")
o.rmempty = false
o.description = translate("Custom Chnroute Lists URL, Click Button Below To Refresh After Edit")
o:value("https://ispip.clang.cn/all_cn.txt", translate("Clang-CN")..translate("(Default)"))
o:value("https://ispip.clang.cn/all_cn_cidr.txt", translate("Clang-CN-CIDR"))
o:value("https://cdn.jsdelivr.net/gh/Hackl0us/GeoIP2-CN@release/CN-ip-cidr.txt", translate("Hackl0us-CN-CIDR")..translate("(Large Size)"))
o.default = "https://ispip.clang.cn/all_cn.txt"

o = s:taboption("chnr_update", Value, "chnr6_custom_url")
o.title = translate("Custom Chnroute6 Lists URL")
o.rmempty = false
o.description = translate("Custom Chnroute6 Lists URL, Click Button Below To Refresh After Edit")
o:value("https://ispip.clang.cn/all_cn_ipv6.txt", translate("Clang-CN-IPV6")..translate("(Default)"))
o.default = "https://ispip.clang.cn/all_cn_ipv6.txt"

o = s:taboption("chnr_update", Button, translate("Chnroute Lists Update")) 
o.title = translate("Update Chnroute Lists")
o.inputtitle = translate("Check And Update")
o.inputstyle = "reload"
o.write = function()
  m.uci:set("openclash", "config", "enable", 1)
  m.uci:commit("openclash")
  SYS.call("/usr/share/openclash/openclash_chnroute.sh >/dev/null 2>&1 &")
  HTTP.redirect(DISP.build_url("admin", "services", "openclash"))
end

o = s:taboption("auto_restart", Flag, "auto_restart", translate("Auto Restart"))
o.description = translate("Auto Restart OpenClash")
o.default=0

o = s:taboption("auto_restart", ListValue, "auto_restart_week_time", translate("Restart Time (Every Week)"))
o:value("*", translate("Every Day"))
o:value("1", translate("Every Monday"))
o:value("2", translate("Every Tuesday"))
o:value("3", translate("Every Wednesday"))
o:value("4", translate("Every Thursday"))
o:value("5", translate("Every Friday"))
o:value("6", translate("Every Saturday"))
o:value("0", translate("Every Sunday"))
o.default=1

o = s:taboption("auto_restart", ListValue, "auto_restart_day_time", translate("Restart time (every day)"))
for t = 0,23 do
o:value(t, t..":00")
end
o.default=0

---- Dashboard Settings
local cn_port=SYS.exec("uci get openclash.config.cn_port 2>/dev/null |tr -d '\n'")
o = s:taboption("dashboard", Value, "cn_port")
o.title = translate("Dashboard Port")
o.default = 9090
o.datatype = "port"
o.rmempty = false
o.description = translate("Dashboard Address Example:").." "..font_green..bold_on..lan_ip.."/luci-static/openclash、"..lan_ip..':'..cn_port..'/ui'..bold_off..font_off

o = s:taboption("dashboard", Value, "dashboard_password")
o.title = translate("Dashboard Secret")
o.rmempty = true
o.description = translate("Set Dashboard Secret")

o = s:taboption("dashboard", Value, "dashboard_forward_domain")
o.title = translate("Public Dashboard Address")
o.datatype = "or(host, string)"
o.placeholder = "example.com"
o.rmempty = true
o.description = translate("Domain Name For Dashboard Login From Public Network")

o = s:taboption("dashboard", Value, "dashboard_forward_port")
o.title = translate("Public Dashboard Port")
o.datatype = "port"
o.rmempty = true
o.description = translate("Port For Dashboard Login From Public Network")

---- version update
core_update = s:taboption("version_update", DummyValue, "", nil)
core_update.template = "openclash/update"

---- debug
o = s:taboption("debug", DummyValue, "", nil)
o.template = "openclash/debug"

---- dlercloud
o = s:taboption("dlercloud", Value, "dler_email")
o.title = translate("Account Email Address")
o.rmempty = true

o = s:taboption("dlercloud", Value, "dler_passwd")
o.title = translate("Account Password")
o.password = true
o.rmempty = true

if m.uci:get("openclash", "config", "dler_token") then
	o = s:taboption("dlercloud", Flag, "dler_checkin")
	o.title = translate("Checkin")
	o.default=0
	o.rmempty = true
end

o = s:taboption("dlercloud", Value, "dler_checkin_interval")
o.title = translate("Checkin Interval (hour)")
o:depends("dler_checkin", "1")
o.default=1
o.rmempty = true

o = s:taboption("dlercloud", Value, "dler_checkin_multiple")
o.title = translate("Checkin Multiple")
o.datatype = "uinteger"
o.default=1
o:depends("dler_checkin", "1")
o.rmempty = true
o.description = font_green..bold_on..translate("Multiple Must Be a Positive Integer and No More Than 50")..bold_off..font_off
function o.validate(self, value)
	if tonumber(value) < 1 then
		return "1"
	end
	if tonumber(value) > 50 then
		return "50"
	end
	return value
end

o = s:taboption("dlercloud", DummyValue, "dler_login", translate("Account Login"))
o.template = "openclash/dler_login"
if m.uci:get("openclash", "config", "dler_token") then
	o.value = font_green..bold_on..translate("Account logged in")..bold_off..font_off
else
	o.value = font_red..bold_on..translate("Account not logged in")..bold_off..font_off
end

-- [[ Edit Server ]] --
s = m:section(TypedSection, "dns_servers", translate("Add Custom DNS Servers")..translate("(Take Effect After Choose Above)"))
s.anonymous = true
s.addremove = true
s.sortable = false
s.template = "cbi/tblsection"
s.rmempty = false

---- enable flag
o = s:option(Flag, "enabled", translate("Enable"), font_red..bold_on..translate("(Enable or Disable)")..bold_off..font_off)
o.rmempty     = false
o.default     = o.enabled
o.cfgvalue    = function(...)
    return Flag.cfgvalue(...) or "1"
end

---- group
o = s:option(ListValue, "group", translate("DNS Server Group"))
o.description = font_red..bold_on..translate("(NameServer Group Must Be Set)")..bold_off..font_off
o:value("nameserver", translate("NameServer"))
o:value("fallback", translate("FallBack"))
o.default     = "nameserver"
o.rempty      = false

---- IP address
o = s:option(Value, "ip", translate("DNS Server Address"))
o.description = font_red..bold_on..translate("(Do Not Add Type Ahead)")..bold_off..font_off
o.placeholder = translate("Not Null")
o.datatype = "or(host, string)"
o.rmempty = true

---- port
o = s:option(Value, "port", translate("DNS Server Port"))
o.description = font_red..bold_on..translate("(Require When Use Non-Standard Port)")..bold_off..font_off
o.datatype    = "port"
o.rempty      = true

---- type
o = s:option(ListValue, "type", translate("DNS Server Type"))
o.description = font_red..bold_on..translate("(Communication protocol)")..bold_off..font_off
o:value("udp", translate("UDP"))
o:value("tcp", translate("TCP"))
o:value("tls", translate("TLS"))
o:value("https", translate("HTTPS"))
o.default     = "udp"
o.rempty      = false

-- [[ Other Rules Manage ]]--
ss = m:section(TypedSection, "other_rules", translate("Other Rules Edit")..translate("(Take Effect After Choose Above)"))
ss.anonymous = true
ss.addremove = true
ss.sortable = true
ss.template = "cbi/tblsection"
ss.extedit = luci.dispatcher.build_url("admin/services/openclash/other-rules-edit/%s")
function ss.create(...)
	local sid = TypedSection.create(...)
	if sid then
		luci.http.redirect(ss.extedit % sid)
		return
	end
end

o = ss:option(Flag, "enabled", translate("Enable"))
o.rmempty     = false
o.default     = o.enabled
o.cfgvalue    = function(...)
    return Flag.cfgvalue(...) or "1"
end

o = ss:option(DummyValue, "config", translate("Config File"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end

o = ss:option(DummyValue, "rule_name", translate("Other Rules Name"))
function o.cfgvalue(...)
	if Value.cfgvalue(...) == "lhie1" then
		return translate("lhie1 Rules")
	elseif Value.cfgvalue(...) == "ConnersHua" then
		return translate("ConnersHua(Provider-type) Rules")
	elseif Value.cfgvalue(...) == "ConnersHua_return" then
		return translate("ConnersHua Return Rules")
	else
		return translate("None")
	end
end

o = ss:option(DummyValue, "Note", translate("Note"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end

-- [[ Edit Authentication ]] --
s = m:section(TypedSection, "authentication", translate("Set Authentication of SOCKS5/HTTP(S)"))
s.anonymous = true
s.addremove = true
s.sortable = false
s.template = "cbi/tblsection"
s.rmempty = false

---- enable flag
o = s:option(Flag, "enabled", translate("Enable"))
o.rmempty     = false
o.default     = o.enabled
o.cfgvalue    = function(...)
    return Flag.cfgvalue(...) or "1"
end

---- username
o = s:option(Value, "username", translate("Username"))
o.placeholder = translate("Not Null")
o.rempty      = true

---- password
o = s:option(Value, "password", translate("Password"))
o.placeholder = translate("Not Null")
o.rmempty = true

if op_mode == "redir-host" then
s = m:section(NamedSection, "config", translate("Set Custom Hosts, Only Work with Redir-Host Mode"))
s.anonymous = true

custom_hosts = s:option(Value, "custom_hosts")
custom_hosts.template = "cbi/tvalue"
custom_hosts.description = translate("Custom Hosts Here, For More Go:").." "..'<a href="https://lancellc.gitbook.io/clash/clash-config-file/dns/host">https://lancellc.gitbook.io/clash/clash-config-file/dns/host</a>'
custom_hosts.rows = 20
custom_hosts.wrap = "off"

function custom_hosts.cfgvalue(self, section)
	return NXFS.readfile("/etc/openclash/custom/openclash_custom_hosts.list") or ""
end
function custom_hosts.write(self, section, value)
	if value then
		value = value:gsub("\r\n?", "\n")
		local old_value = NXFS.readfile("/etc/openclash/custom/openclash_custom_hosts.list")
	  if value ~= old_value then
			NXFS.writefile("/etc/openclash/custom/openclash_custom_hosts.list", value)
		end
	end
end
end

local t = {
    {Commit, Apply}
}

a = m:section(Table, t)

o = a:option(Button, "Commit", " ")
o.inputtitle = translate("Commit Settings")
o.inputstyle = "apply"
o.write = function()
  m.uci:commit("openclash")
end

o = a:option(Button, "Apply", " ")
o.inputtitle = translate("Apply Settings")
o.inputstyle = "apply"
o.write = function()
  m.uci:set("openclash", "config", "enable", 1)
  m.uci:commit("openclash")
  SYS.call("/etc/init.d/openclash restart >/dev/null 2>&1 &")
  HTTP.redirect(DISP.build_url("admin", "services", "openclash"))
end

m:append(Template("openclash/config_editor"))
m:append(Template("openclash/toolbar_show"))

return m


