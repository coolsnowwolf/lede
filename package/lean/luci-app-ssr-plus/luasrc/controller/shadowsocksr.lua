-- Copyright (C) 2017 yushi studio <ywb94@qq.com>
-- Licensed to the public under the GNU General Public License v3.

module("luci.controller.shadowsocksr", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/shadowsocksr") then
		return
	end

  entry({"admin", "services", "shadowsocksr"},alias("admin", "services", "shadowsocksr", "client"),_("ShadowSocksR Plus+"), 10).acl_depends = { "luci-app-ssr-plus" }

  entry({"admin", "services", "shadowsocksr", "client"},cbi("shadowsocksr/client"),_("SSR Client"), 10).leaf = true
 
	entry({"admin", "services", "shadowsocksr", "servers"}, arcombine(cbi("shadowsocksr/servers"), cbi("shadowsocksr/client-config")),_("Severs Nodes"), 20).leaf = true
	
	entry({"admin", "services", "shadowsocksr", "control"},cbi("shadowsocksr/control"),_("Access Control"), 30).leaf = true
	
	entry({"admin", "services", "shadowsocksr", "list"},form("shadowsocksr/list"),_("GFW List"), 40).leaf = true
	
	entry({"admin", "services", "shadowsocksr", "advanced"},cbi("shadowsocksr/advanced"),_("Advanced Settings"), 50).leaf = true
	
	if nixio.fs.access("/usr/bin/ssr-server") then
	    entry({"admin", "services", "shadowsocksr", "server"},arcombine(cbi("shadowsocksr/server"), cbi("shadowsocksr/server-config")),_("SSR Server"), 60).leaf = true
	
	end
	
	entry({"admin", "services", "shadowsocksr", "status"},form("shadowsocksr/status"),_("Status"), 70).leaf = true
	
	entry({"admin", "services", "shadowsocksr", "check"}, call("check_status"))
	entry({"admin", "services", "shadowsocksr", "refresh"}, call("refresh_data"))
	entry({"admin", "services", "shadowsocksr", "checkport"}, call("check_port"))
	entry({"admin", "services", "shadowsocksr", "subscribe"}, call("subscribe_nodes"))
	
	entry({"admin", "services", "shadowsocksr","run"},call("act_status")).leaf=true
	
	entry({"admin", "services", "shadowsocksr", "ping"}, call("act_ping")).leaf=true
	
	entry({"admin", "services", "shadowsocksr", "fileread"}, call("act_read"), nil).leaf=true

	entry({"admin", "services", "shadowsocksr", "logview"}, cbi("shadowsocksr/logview", {hideapplybtn=true, hidesavebtn=true, hideresetbtn=true}), _("Log") ,80).leaf=true

end

function act_status()
  local e={}
  e.running=luci.sys.call("busybox ps -w | grep ' /var/etc/shadowsocksr' | grep -v grep >/dev/null")==0
  luci.http.prepare_content("application/json")
  luci.http.write_json(e)
end

function act_ping()
	local e={}
	e.index=luci.http.formvalue("index")
	e.ping=luci.sys.exec("ping -c 1 -W 1 %q 2>&1 | grep -o 'time=[0-9]*.[0-9]' | awk -F '=' '{print$2}'"%luci.http.formvalue("domain"))
	local domain = luci.http.formvalue("domain")
	local port = luci.http.formvalue("port")
	local iret = luci.sys.call('ipset -q add ss_spec_wan_ac "%s" 2>/dev/null' % domain)
	local socket = nixio.socket("inet", "stream")
	socket:setopt("socket", "rcvtimeo", 3)
	socket:setopt("socket", "sndtimeo", 3)
	e.socket = socket:connect(domain, port)
	socket:close()
	if (iret == 0) then
		luci.sys.call('ipset -q del ss_spec_wan_ac "%s"' % domain)
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end

function check_status()
local sret=luci.sys.call("tcping -q -c 1 -i 3 -p 80 www.%s.com" % luci.http.formvalue("set"))
if sret== 0 then
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
  refresh_cmd="wget-ssl --no-check-certificate https://raw.githubusercontent.com/gfwlist/gfwlist/master/gfwlist.txt -O /tmp/gfw.b64"
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
    luci.sys.exec("cp -f /tmp/gfwnew.txt /etc/dnsmasq.ssr/gfw_list.conf && cp -f /tmp/gfwnew.txt /tmp/dnsmasq.ssr/gfw_list.conf")
    luci.sys.call("/etc/init.d/dnsmasq restart")
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
  oldcount=luci.sys.exec("cat /etc/ssr/china_ssr.txt | wc -l")
  if tonumber(icount) ~= tonumber(oldcount) then
   luci.sys.exec("cp -f /tmp/china_ssr.txt /etc/ssr/china_ssr.txt")
   luci.sys.call("/usr/share/shadowsocksr/chinaipset.sh")
   retstring=tostring(tonumber(icount))
  else
   retstring ="0"
  end

 else
  retstring ="-1"
 end
 luci.sys.exec("rm -f /tmp/china_ssr.txt ")
elseif set == "ads_data" then
  local need_process = 0
  if nixio.fs.access("/usr/bin/wget-ssl") then
  refresh_cmd="wget-ssl --no-check-certificate -O - https://easylist-downloads.adblockplus.org/easylistchina+easylist.txt > /tmp/adnew.conf"
  need_process = 1
 else
  refresh_cmd="wget -O /tmp/ad.conf http://iytc.net/tools/ad.conf"
 end
 sret=luci.sys.call(refresh_cmd .. " 2>/dev/null")
 if sret== 0 then
  if need_process > 0 then
    luci.sys.call("/usr/bin/ssr-ad")
  end
  icount = luci.sys.exec("cat /tmp/ad.conf | wc -l")
  if tonumber(icount) > 1000 then
   oldcount=luci.sys.exec("cat /etc/dnsmasq.ssr/ad.conf | wc -l")
   if tonumber(icount) ~= tonumber(oldcount) then
    luci.sys.exec("cp -f /tmp/ad.conf /etc/dnsmasq.ssr/ad.conf && cp -f /tmp/ad.conf /tmp/dnsmasq.ssr/ad.conf")
    retstring=tostring(math.ceil(tonumber(icount)))
    luci.sys.call("/etc/init.d/dnsmasq restart")
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
else
 retstring ="-1"
end	
luci.http.prepare_content("application/json")
luci.http.write_json({ ret=retstring ,retcount=icount})
end


function check_port()
local set = luci.http.formvalue("set")
local retstring="<br /><br />"
local s
local server_name = ""
local shadowsocksr = "shadowsocksr"
local uci = luci.model.uci.cursor()
local iret=1

if set == "nslook" then
retstring = luci.sys.exec("/usr/bin/nslookup www.google.com 127.0.0.1#5353 2>&1")
-- domains = {}
-- domains.push("www.google.com")
-- retjson = luci.util.ubus("network.rrdns", "lookup", {addrs={domains}, timerout=3000})
else
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
	retstring =retstring .. '<font color="green">[%s] OK.</font><br />' % server_name
	else
	retstring =retstring .. '<font color="red">[%s] Error.</font><br />' % server_name
	end	
	if  iret== 0 then
	luci.sys.call(" ipset del ss_spec_wan_ac " .. s.server)
	end
end)
end

luci.http.prepare_content("application/json")
luci.http.write_json({ ret=retstring })
end

function subscribe_nodes()
	local iret=255
	iret = luci.sys.call("/usr/bin/lua /usr/share/shadowsocksr/subscribe_nodes.lua  >> /tmp/ssrplus.log 2>&1")
	luci.http.prepare_content("application/json")
	luci.http.write_json({ ret = tostring(iret) })
end

-- called by XHR.get from logview.htm
function act_read(lfile)
	local fs = require "nixio.fs"
	local http = require "luci.http"
	local lfile = http.formvalue("lfile")
	local ldata={}
	ldata[#ldata+1] = fs.readfile(lfile) or "_nofile_"
	if ldata[1] == "" then
		ldata[1] = "_nodata_"
	end
	http.prepare_content("application/json")
	http.write_json(ldata)
end
