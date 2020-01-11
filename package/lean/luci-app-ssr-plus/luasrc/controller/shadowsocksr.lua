-- Copyright (C) 2017 yushi studio <ywb94@qq.com>
-- Licensed to the public under the GNU General Public License v3.

module("luci.controller.shadowsocksr", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/shadowsocksr") then
		return
	end


	entry({"admin", "services", "shadowsocksr"}, alias("admin", "services", "shadowsocksr", "client"),_("ShadowSocksR Plus+"), 10).dependent = true

	entry({"admin", "services", "shadowsocksr", "client"}, cbi("shadowsocksr/client"),_("SSR Client"), 10).leaf = true

	entry({"admin", "services", "shadowsocksr", "servers"}, arcombine(cbi("shadowsocksr/servers", {autoapply=true}), cbi("shadowsocksr/client-config")),_("Severs Nodes"), 20).leaf = true

	entry({"admin", "services", "shadowsocksr", "control"},cbi("shadowsocksr/control"), _("Access Control"), 30).leaf = true

	-- entry({"admin", "services", "shadowsocksr", "list"},form("shadowsocksr/list"),_("GFW List"), 40).leaf = true

	entry({"admin", "services", "shadowsocksr", "advanced"},cbi("shadowsocksr/advanced"),_("Advanced Settings"), 50).leaf = true

	if nixio.fs.access("/usr/bin/ssr-server") then
		entry({"admin", "services", "shadowsocksr", "server"},arcombine(cbi("shadowsocksr/server"), cbi("shadowsocksr/server-config")),_("SSR Server"), 60).leaf = true
	end

	entry({"admin", "services", "shadowsocksr", "status"},form("shadowsocksr/status"),_("Status"), 70).leaf = true

	entry({"admin", "services", "shadowsocksr", "check"}, call("check_status"))
	entry({"admin", "services", "shadowsocksr", "refresh"}, call("refresh_data"))
	entry({"admin", "services", "shadowsocksr", "subscribe"}, call("subscribe"))

	entry({"admin", "services", "shadowsocksr", "checkport"}, call("check_port"))

	entry({"admin", "services", "shadowsocksr", "log"},form("shadowsocksr/log"),_("Log"), 80).leaf = true

	entry({"admin", "services", "shadowsocksr","run"},call("act_status")).leaf=true

	entry({"admin", "services", "shadowsocksr", "ping"}, call("act_ping")).leaf=true

end

function subscribe()
	luci.sys.call("/usr/bin/lua /usr/share/shadowsocksr/subscribe.lua  >> /tmp/ssrplus.log 2>&1")
	luci.http.prepare_content("application/json")
	luci.http.write_json({ ret = 1 })
end

function act_status()
	local e={}
	e.running=luci.sys.call("busybox ps -w | grep ssr-retcp | grep -v grep >/dev/null")==0
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end

function act_ping()
	local e = {}
	local domain = luci.http.formvalue("domain")
	local port = luci.http.formvalue("port")
	e.index = luci.http.formvalue("index")
	e.ping = luci.sys.exec("ping -c 1 -W 1 %q 2>&1 | grep -o 'time=[0-9]*.[0-9]' | awk -F '=' '{print$2}'" % domain)

	local iret = luci.sys.call(" ipset add ss_spec_wan_ac " .. domain .. " 2>/dev/null")
	local socket = nixio.socket("inet", "stream")
	socket:setopt("socket", "rcvtimeo", 3)
	socket:setopt("socket", "sndtimeo", 3)
	e.socket = socket:connect(domain, port)
	socket:close()
	if (iret == 0) then
		luci.sys.call(" ipset del ss_spec_wan_ac " .. domain)
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end

function check_status()
	local set = "/usr/bin/ssr-check www." .. luci.http.formvalue("set") .. ".com 80 3 1"
	sret = luci.sys.call(set)
	if sret == 0 then
		retstring ="0"
	else
		retstring ="1"
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json({ ret=retstring })
end

function refresh_data()
local set =luci.http.formvalue("set")
local icount =0

if set == "gfw_data" then
 if nixio.fs.access("/usr/bin/wget-ssl") then
	refresh_cmd="wget-ssl --no-check-certificate https://cdn.jsdelivr.net/gh/gfwlist/gfwlist/gfwlist.txt -O /tmp/gfw.b64"
 else
	refresh_cmd="wget -O /tmp/gfw.b64 http://iytc.net/tools/list.b64"
 end
 sret=luci.sys.call(refresh_cmd .. " 2>/dev/null")
 if sret== 0 then
	luci.sys.call("/usr/bin/ssr-gfw")
	icount = luci.sys.exec("cat /tmp/gfwnew.txt | wc -l")
	if tonumber(icount)>1000 then
	 oldcount=luci.sys.exec("cat /etc/dnsmasq.ssr/gfw_list.conf | wc -l")
	 if tonumber(icount) ~= tonumber(oldcount) then
		luci.sys.exec("cp -f /tmp/gfwnew.txt /etc/dnsmasq.ssr/gfw_list.conf")
		retstring=tostring(math.ceil(tonumber(icount)/2))
	 else
		retstring ="0"
	 end
	else
	 retstring ="-1"
	end
	luci.sys.exec("rm -f /tmp/gfwnew.txt ")
 else
	retstring ="-1"
 end
elseif set == "ip_data" then
 refresh_cmd="wget -O- 'http://ftp.apnic.net/apnic/stats/apnic/delegated-apnic-latest'  2>/dev/null| awk -F\\| '/CN\\|ipv4/ { printf(\"%s/%d\\n\", $4, 32-log($5)/log(2)) }' > /tmp/china_ssr.txt"
 sret=luci.sys.call(refresh_cmd)
 icount = luci.sys.exec("cat /tmp/china_ssr.txt | wc -l")
 if  sret== 0 and tonumber(icount)>1000 then
	oldcount=luci.sys.exec("cat /etc/china_ssr.txt | wc -l")
	if tonumber(icount) ~= tonumber(oldcount) then
	 luci.sys.exec("cp -f /tmp/china_ssr.txt /etc/china_ssr.txt")
	 retstring=tostring(tonumber(icount))
	else
	 retstring ="0"
	end

 else
	retstring ="-1"
 end
 luci.sys.exec("rm -f /tmp/china_ssr.txt ")
else
	local need_process = 0
	if nixio.fs.access("/usr/bin/wget-ssl") then
	refresh_cmd="wget-ssl --no-check-certificate -O - https://easylist-downloads.adblockplus.org/easylistchina+easylist.txt > /tmp/adnew.conf"
	need_process = 1
 else
	refresh_cmd="wget -O /tmp/ad.conf http://iytc.net/tools/ad.conf"
 end
 sret=luci.sys.call(refresh_cmd .. " 2>/dev/null")
 if sret== 0 then
	if need_process == 1 then
		luci.sys.call("/usr/bin/ssr-ad")
	end
	icount = luci.sys.exec("cat /tmp/ad.conf | wc -l")
	if tonumber(icount)>1000 then
	 if nixio.fs.access("/etc/dnsmasq.ssr/ad.conf") then
		oldcount=luci.sys.exec("cat /etc/dnsmasq.ssr/ad.conf | wc -l")
	 else
		oldcount=0
	 end

	 if tonumber(icount) ~= tonumber(oldcount) then
		luci.sys.exec("cp -f /tmp/ad.conf /etc/dnsmasq.ssr/ad.conf")
		retstring=tostring(math.ceil(tonumber(icount)))
		if oldcount==0 then
		 luci.sys.call("/etc/init.d/dnsmasq restart")
		end
	 else
		retstring ="0"
	 end
	else
	 retstring ="-1"
	end
	luci.sys.exec("rm -f /tmp/ad.conf ")
 else
	retstring ="-1"
 end
end
luci.http.prepare_content("application/json")
luci.http.write_json({ ret=retstring ,retcount=icount})
end


function check_port()
local set=""
local retstring="<br /><br />"
local s
local server_name = ""
local shadowsocksr = "shadowsocksr"
local uci = luci.model.uci.cursor()
local iret=1

uci:foreach(shadowsocksr, "servers", function(s)

	if s.alias then
		server_name=s.alias
	elseif s.server and s.server_port then
		server_name= "%s:%s" %{s.server, s.server_port}
	end
	iret=luci.sys.call(" ipset add ss_spec_wan_ac " .. s.server .. " 2>/dev/null")
	socket = nixio.socket("inet", "stream")
	socket:setopt("socket", "rcvtimeo", 3)
	socket:setopt("socket", "sndtimeo", 3)
	ret=socket:connect(s.server,s.server_port)
	if  tostring(ret) == "true" then
	socket:close()
	retstring =retstring .. "<font color='green'>[" .. server_name .. "] OK.</font><br />"
	else
	retstring =retstring .. "<font color='red'>[" .. server_name .. "] Error.</font><br />"
	end
	if  iret== 0 then
	luci.sys.call(" ipset del ss_spec_wan_ac " .. s.server)
	end
end)

luci.http.prepare_content("application/json")
luci.http.write_json({ ret=retstring })
end
