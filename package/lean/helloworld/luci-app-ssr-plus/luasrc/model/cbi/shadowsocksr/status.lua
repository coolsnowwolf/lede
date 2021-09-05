-- Copyright (C) 2017 yushi studio <ywb94@qq.com>
-- Licensed to the public under the GNU General Public License v3.
require "nixio.fs"
require "luci.sys"
require "luci.model.uci"
local m, s, o
local redir_run = 0
local reudp_run = 0
local sock5_run = 0
local server_run = 0
local kcptun_run = 0
local tunnel_run = 0
local gfw_count = 0
local ad_count = 0
local ip_count = 0
local nfip_count = 0
local Process_list = luci.sys.exec("busybox ps -w")
local uci = luci.model.uci.cursor()
-- html constants
font_blue = [[<font color="green">]]
font_off = [[</font>]]
bold_on = [[<strong>]]
bold_off = [[</strong>]]
local kcptun_version = translate("Unknown")
local kcp_file = "/usr/bin/kcptun-client"
if not nixio.fs.access(kcp_file) then
	kcptun_version = translate("Not exist")
else
	if not nixio.fs.access(kcp_file, "rwx", "rx", "rx") then
		nixio.fs.chmod(kcp_file, 755)
	end
	kcptun_version = luci.sys.exec(kcp_file .. " -v | awk '{printf $3}'")
	if not kcptun_version or kcptun_version == "" then
		kcptun_version = translate("Unknown")
	end
end

if nixio.fs.access("/etc/ssrplus/gfw_list.conf") then
	gfw_count = tonumber(luci.sys.exec("cat /etc/ssrplus/gfw_list.conf | wc -l")) / 2
end

if nixio.fs.access("/etc/ssrplus/ad.conf") then
	ad_count = tonumber(luci.sys.exec("cat /etc/ssrplus/ad.conf | wc -l"))
end

if nixio.fs.access("/etc/ssrplus/china_ssr.txt") then
	ip_count = tonumber(luci.sys.exec("cat /etc/ssrplus/china_ssr.txt | wc -l"))
end

if nixio.fs.access("/etc/ssrplus/netflixip.list") then
	nfip_count = tonumber(luci.sys.exec("cat /etc/ssrplus/netflixip.list | wc -l"))
end

if Process_list:find("udp.only.ssr.reudp") then
	reudp_run = 1
end

if Process_list:find("tcp.only.ssr.retcp") then
	redir_run = 1
end

if Process_list:find("tcp.udp.ssr.local") then
	sock5_run = 1
end

if Process_list:find("tcp.udp.ssr.retcp") then
	redir_run = 1
	reudp_run = 1
end

if Process_list:find("local.ssr.retcp") then
	redir_run = 1
	sock5_run = 1
end

if Process_list:find("local.udp.ssr.retcp") then
	reudp_run = 1
	redir_run = 1
	sock5_run = 1
end

if Process_list:find("kcptun.client") then
	kcptun_run = 1
end

if Process_list:find("ssr.server") then
	server_run = 1
end

if Process_list:find("ssrplus/bin/pdnsd") or (Process_list:find("ssrplus.dns") and Process_list:find("dns2socks.127.0.0.1.*127.0.0.1.5335")) then
	pdnsd_run = 1
end

m = SimpleForm("Version")
m.reset = false
m.submit = false

s = m:field(DummyValue, "redir_run", translate("Global Client"))
s.rawhtml = true
if redir_run == 1 then
	s.value = font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else
	s.value = translate("Not Running")
end

s = m:field(DummyValue, "reudp_run", translate("Game Mode UDP Relay"))
s.rawhtml = true
if reudp_run == 1 then
	s.value = font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else
	s.value = translate("Not Running")
end

if uci:get_first("shadowsocksr", 'global', 'pdnsd_enable', '0') ~= '0' then
	s = m:field(DummyValue, "pdnsd_run", translate("DNS Anti-pollution"))
	s.rawhtml = true
	if pdnsd_run == 1 then
		s.value = font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
	else
		s.value = translate("Not Running")
	end
end

s = m:field(DummyValue, "sock5_run", translate("Global SOCKS5 Proxy Server"))
s.rawhtml = true
if sock5_run == 1 then
	s.value = font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else
	s.value = translate("Not Running")
end

s = m:field(DummyValue, "server_run", translate("Local Servers"))
s.rawhtml = true
if server_run == 1 then
	s.value = font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else
	s.value = translate("Not Running")
end

if nixio.fs.access("/usr/bin/kcptun-client") then
	s = m:field(DummyValue, "kcp_version", translate("KcpTun Version"))
	s.rawhtml = true
	s.value = kcptun_version
	s = m:field(DummyValue, "kcptun_run", translate("KcpTun"))
	s.rawhtml = true
	if kcptun_run == 1 then
		s.value = font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
	else
		s.value = translate("Not Running")
	end
end

s = m:field(DummyValue, "google", translate("Google Connectivity"))
s.value = translate("No Check")
s.template = "shadowsocksr/check"

s = m:field(DummyValue, "baidu", translate("Baidu Connectivity"))
s.value = translate("No Check")
s.template = "shadowsocksr/check"

s = m:field(DummyValue, "gfw_data", translate("GFW List Data"))
s.rawhtml = true
s.template = "shadowsocksr/refresh"
s.value = gfw_count .. " " .. translate("Records")

s = m:field(DummyValue, "ip_data", translate("China IP Data"))
s.rawhtml = true
s.template = "shadowsocksr/refresh"
s.value = ip_count .. " " .. translate("Records")

if uci:get_first("shadowsocksr", 'global', 'netflix_enable', '0') ~= '0' then
s = m:field(DummyValue, "nfip_data", translate("Netflix IP Data"))
s.rawhtml = true
s.template = "shadowsocksr/refresh"
s.value = nfip_count .. " " .. translate("Records")
end

if uci:get_first("shadowsocksr", 'global', 'adblock', '0') == '1' then
	s = m:field(DummyValue, "ad_data", translate("Advertising Data"))
	s.rawhtml = true
	s.template = "shadowsocksr/refresh"
	s.value = ad_count .. " " .. translate("Records")
end

if uci:get_first("shadowsocksr", 'global', 'pdnsd_enable', '0') == '1' then
	s = m:field(DummyValue, "cache", translate("Reset pdnsd cache"))
	s.template = "shadowsocksr/cache"
end

s = m:field(DummyValue, "check_port", translate("Check Server Port"))
s.template = "shadowsocksr/checkport"
s.value = translate("No Check")

return m
