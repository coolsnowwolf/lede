-- Copyright (C) 2017 yushi studio <ywb94@qq.com>
-- Licensed to the public under the GNU General Public License v3.

local IPK_Version="20191116.118"
local m, s, o
local redir_run=0
local reudp_run=0
local sock5_run=0
local ssock5_run=0
local v2sock5_run=0
local server_run=0
local sserver_run=0
local v2server_run=0
local kcptun_run=0
local tunnel_run=0
local udp2raw_run=0
local udpspeeder_run=0
local gfw_count=0
local ad_count=0
local ip_count=0
local gfwmode=0
local pdnsd_run=0
local dnsforwarder_run=0
local dnscrypt_proxy_run=0
local haproxy_run=0
local privoxy_run=0

if nixio.fs.access("/etc/dnsmasq.ssr/gfw_list.conf") then
gfwmode=1		
end

local shadowsocksr = "shadowsocksr"
-- html constants
font_blue = [[<font color="green">]]
font_off = [[</font>]]
bold_on  = [[<strong>]]
bold_off = [[</strong>]]

local fs = require "nixio.fs"
local sys = require "luci.sys"
local kcptun_version=translate("Unknown")
local kcp_file="/usr/bin/kcptun-client"
if not fs.access(kcp_file)  then
 kcptun_version=translate("Not exist")
else
 if not fs.access(kcp_file, "rwx", "rx", "rx") then
   fs.chmod(kcp_file, 755)
 end
 kcptun_version=sys.exec(kcp_file .. " -v | awk '{printf $3}'")
 if not kcptun_version or kcptun_version == "" then
     kcptun_version = translate("Unknown")
 end
        
end

local udp2raw_version=translate("Unknown")
local udp2raw_file="/usr/bin/udp2raw"
if not fs.access(udp2raw_file) then
	udp2raw_version=translate("Not exist")
else
	if not fs.access(udp2raw_file, "rwx", "rx", "rx") then
		fs.chmod(udp2raw_file, 755)
	end
	udp2raw_version=sys.exec(udp2raw_file .. " -h |grep 'git version' |awk -F ':' '{print $2}'|awk '{print $1}'")
	if not udp2raw_version or udp2raw_version == "" then
		udp2raw_version = translate("Unknown")
	end
end

local udpspeeder_version=translate("Unknown")
local udpspeeder_file="/usr/bin/udpspeeder"
if not fs.access(udpspeeder_file) then
	udpspeeder_version=translate("Not exist")
else
	if not fs.access(udpspeeder_file, "rwx", "rx", "rx") then
		fs.chmod(udpspeeder_file, 755)
	end
	udpspeeder_version=sys.exec(udpspeeder_file .. " -h |grep 'git version' |awk -F ':' '{print $2}'|awk '{print $1}'")
	if not udpspeeder_version or udpspeeder_version == "" then
		udpspeeder_version = translate("Unknown")
	end
end

if gfwmode==1 then 
 gfw_count = tonumber(sys.exec("cat /etc/dnsmasq.ssr/gfw_list.conf | wc -l"))/2
 if nixio.fs.access("/etc/dnsmasq.ssr/ad.conf") then
  ad_count=tonumber(sys.exec("cat /etc/dnsmasq.ssr/ad.conf | wc -l"))
 end
end
 
if nixio.fs.access("/etc/china_ssr.txt") then 
 ip_count = sys.exec("cat /etc/china_ssr.txt | wc -l")
end

local icount=sys.exec("ps -w | grep ssr-reudp |grep -v grep| wc -l")
if tonumber(icount)>0 then
reudp_run=1
else
icount=sys.exec("ps -w | grep ssr-retcp |grep \"\\-u\"|grep -v grep| wc -l")
if tonumber(icount)>0 then
reudp_run=1
end
end


if luci.sys.call("ps -w | grep ssr-retcp | grep -v grep >/dev/null") == 0 then
redir_run=1
end	

if luci.sys.call("pidof ssr-local >/dev/null") == 0 then
sock5_run=1
end

if luci.sys.call("pidof ss-local >/dev/null") == 0 then
ssock5_run=1
end

if luci.sys.call("ps -w | grep v2-ssr-local | grep -v grep >/dev/null") == 0 then
v2sock5_run=1
end

if luci.sys.call("pidof privoxy >/dev/null") == 0 then
privoxy_run=1
end

if luci.sys.call("pidof kcptun-client >/dev/null") == 0 then
kcptun_run=1
end	

if luci.sys.call("pidof ssr-server >/dev/null") == 0 then
server_run=1
end

if luci.sys.call("pidof ss-server >/dev/null") == 0 then
sserver_run=1
end

if luci.sys.call("ps -w | grep v2ray-server | grep -v grep >/dev/null") == 0 then
v2server_run=1
end	

if luci.sys.call("ps -w | grep ssr-tunnel |grep -v grep >/dev/null") == 0 then
tunnel_run=1
end

if luci.sys.call("pidof udp2raw >/dev/null") == 0 then
udp2raw_run=1
end

if luci.sys.call("pidof udpspeeder >/dev/null") == 0 then
udpspeeder_run=1
end	

if luci.sys.call("pidof pdnsd >/dev/null") == 0 then                 
pdnsd_run=1     
end	

if luci.sys.call("pidof dnsparsing >/dev/null") == 0 then                 
dnsforwarder_run=1     
end

if luci.sys.call("pidof dnscrypt-proxy >/dev/null") == 0 then                 
dnscrypt_proxy_run=1     
end

if luci.sys.call("pidof haproxy >/dev/null") == 0 then                 
haproxy_run=1     
end	

m = SimpleForm("Version")
m.reset = false
m.submit = false

s=m:field(DummyValue,"redir_run",translate("Global Client")) 
s.rawhtml  = true
if redir_run == 1 then
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else
s.value = translate("Not Running")
end

s=m:field(DummyValue,"reudp_run",translate("Game Mode UDP Relay")) 
s.rawhtml  = true
if reudp_run == 1 then
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else
s.value = translate("Not Running")
end

s=m:field(DummyValue,"haproxy_run",translate("Load Balancing")) 
s.rawhtml  = true
if haproxy_run == 1 then
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else
s.value = translate("Not Running")
end

s=m:field(DummyValue,"pdnsd_run",translate("PDNSD"))
s.rawhtml  = true                                              
if pdnsd_run == 1 then                             
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else             
s.value = translate("Not Running")
end 

s=m:field(DummyValue,"dnsforwarder_run",translate("dnsforwarder"))
s.rawhtml  = true                                              
if dnsforwarder_run == 1 then                             
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else             
s.value = translate("Not Running")
end

s=m:field(DummyValue,"dnscrypt_proxy_run",translate("dnscrypt_proxy"))
s.rawhtml  = true                                              
if dnscrypt_proxy_run == 1 then                             
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else             
s.value = translate("Not Running")
end 




if nixio.fs.access("/usr/bin/ssr-local") then
s=m:field(DummyValue,"sock5_run",translate("SOCKS5 Proxy")) 
s.rawhtml  = true
if sock5_run == 1 then
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else
s.value = translate("Not Running")
end
end

if nixio.fs.access("/usr/bin/ss-local") then
s=m:field(DummyValue,"ssock5_run",translate("SSOCKS5 Proxy")) 
s.rawhtml  = true
if ssock5_run == 1 then
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else
s.value = translate("Not Running")
end
end

if nixio.fs.access("/usr/bin/v2ray/v2ray") then
s=m:field(DummyValue,"ssock5_run",translate("V2SOCKS5 Proxy")) 
s.rawhtml  = true
if v2sock5_run == 1 then
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else
s.value = translate("Not Running")
end
end

if nixio.fs.access("/usr/sbin/privoxy") then
s=m:field(DummyValue,"privoxy_run",translate("HTTP Proxy")) 
s.rawhtml  = true
if privoxy_run == 1 then
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else
s.value = translate("Not Running")
end
end

if nixio.fs.access("/usr/bin/ssr-server") then
s=m:field(DummyValue,"server_run",translate("Global SSR Server")) 
s.rawhtml  = true
if server_run == 1 then
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else
s.value = translate("Not Running")
end
end

if nixio.fs.access("/usr/bin/ss-server") then
s=m:field(DummyValue,"sserver_run",translate("Global SS Server")) 
s.rawhtml  = true
if sserver_run == 1 then
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else
s.value = translate("Not Running")
end
end

if nixio.fs.access("/usr/bin/v2ray") then
s=m:field(DummyValue,"v2server_run",translate("Global V2RAY Server")) 
s.rawhtml  = true
if v2server_run == 1 then
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else
s.value = translate("Not Running")
end
end

if nixio.fs.access("/usr/bin/kcptun-client") then
s=m:field(DummyValue,"kcp_version",translate("KcpTun Version")) 
s.rawhtml  = true
s.value =kcptun_version

s=m:field(DummyValue,"kcptun_run",translate("KcpTun")) 
s.rawhtml  = true
if kcptun_run == 1 then
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else
s.value = translate("Not Running")
end
end
s=m:field(DummyValue,"version",translate("IPK Version")) 
s.rawhtml  = true
s.value =IPK_Version

s=m:field(DummyValue,"udp2raw_version",translate("udp2raw Version")) 
s.rawhtml  = true
s.value =udp2raw_version
s=m:field(DummyValue,"udp2raw_run",translate("udp2raw")) 
s.rawhtml  = true
if udp2raw_run == 1 then
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else
s.value = translate("Not Running")
end
s=m:field(DummyValue,"udp2raw_project",translate("udp2raw tunnel Project")) 
s.rawhtml  = true
s.value =bold_on .. [[<a href="]] .. "https://github.com/wangyu-/udp2raw-tunnel" .. [[" >]]
	.. "https://github.com/wangyu-/udp2raw-tunnel" .. [[</a>]] .. bold_off

s=m:field(DummyValue,"udpspeeder_version",translate("UDPspeeder Version")) 
s.rawhtml  = true
s.value =udpspeeder_version
s=m:field(DummyValue,"udpspeeder_run",translate("UDPspeeder")) 
s.rawhtml  = true
if udpspeeder_run == 1 then
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else
s.value = translate("Not Running")
end
s=m:field(DummyValue,"udpspeeder_project",translate("UDPspeeder Project")) 
s.rawhtml  = true
s.value =bold_on .. [[<a href="]] .. "https://github.com/wangyu-/UDPspeeder" .. [[" >]]
	.. "https://github.com/wangyu-/UDPspeeder" .. [[</a>]] .. bold_off
	

if gfwmode==1 then 
s=m:field(DummyValue,"gfw_data",translate("GFW List Data")) 
s.rawhtml  = true
s.template = "shadowsocksr/refresh"
s.value =tostring(math.ceil(gfw_count)) .. " " .. translate("Records")


s=m:field(DummyValue,"ad_data",translate("Advertising Data")) 
s.rawhtml  = true
s.template = "shadowsocksr/refresh"
s.value =tostring(math.ceil(ad_count)) .. " " .. translate("Records")
end
s=m:field(DummyValue,"ip_data",translate("China IP Data")) 
s.rawhtml  = true
s.template = "shadowsocksr/refresh"
s.value =ip_count .. " " .. translate("Records")

s=m:field(DummyValue,"check_port",translate("Check Server Port"))
s.template = "shadowsocksr/checkport"
s.value =translate("No Check")



return m
