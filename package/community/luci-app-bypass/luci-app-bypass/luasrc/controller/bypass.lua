module("luci.controller.bypass",package.seeall)
local http = require "luci.http"
local api = require "luci.model.cbi.bypass.api"
local xray = require "luci.model.cbi.bypass.xray"
local trojan_go = require "luci.model.cbi.bypass.trojan_go"
function index()
	if not nixio.fs.access("/etc/config/bypass") then
		return
	end
	local e=entry({"admin","services","bypass"},firstchild(),_("Bypass"),2)
	e.dependent=false
	e.acl_depends={ "luci-app-bypass" }
	entry({"admin","services","bypass","base"},cbi("bypass/base"),_("Base Setting"),1).leaf=true
	entry({"admin","services","bypass","servers"},arcombine(cbi("bypass/servers",{autoapply=true}),cbi("bypass/client-config")),_("Severs Nodes"),2).leaf=true
	entry({"admin","services","bypass","control"},cbi("bypass/control"),_("Access Control"),3).leaf=true
	entry({"admin","services","bypass","advanced"},cbi("bypass/advanced"),_("Advanced Settings"),4).leaf=true
	entry({"admin", "services", "bypass", "app_update"}, cbi("bypass/app_update"), _("App Update"), 5).leaf = true
	if luci.sys.call("which ssr-server >/dev/null")==0 or luci.sys.call("which ss-server >/dev/null")==0 or luci.sys.call("which microsocks >/dev/null")==0 then
	      entry({"admin","services","bypass","server"},arcombine(cbi("bypass/server"),cbi("bypass/server-config")),_("SSR Server"),6).leaf=true
	end
	entry({"admin","services","bypass","status"},form("bypass/status"),_("Status"),7).leaf=true
	entry({"admin","services","bypass","log"},form("bypass/log"),_("Log"),8).leaf=true
	entry({"admin","services","bypass","check"},call("check_status"))
	entry({"admin","services","bypass","refresh"},call("refresh_data"))
	entry({"admin","services","bypass","subscribe"},call("subscribe"))
	entry({"admin","services","bypass","checkport"},call("check_port"))
	entry({"admin","services","bypass","run"},call("act_status"))
	entry({"admin","services","bypass","ping"},call("act_ping"))
	entry({"admin", "services", "bypass", "xray_check"}, call("xray_check")).leaf = true
	entry({"admin", "services", "bypass", "xray_update"}, call("xray_update")).leaf = true
	entry({"admin", "services", "bypass", "v2ray_check"}, call("v2ray_check")).leaf = true
	entry({"admin", "services", "bypass", "v2ray_update"}, call("v2ray_update")).leaf = true
	entry({"admin", "services", "bypass", "trojan_go_check"}, call("trojan_go_check")).leaf = true
	entry({"admin", "services", "bypass", "trojan_go_update"}, call("trojan_go_update")).leaf = true
	entry({'admin', 'services', "bypass", 'ip'}, call('check_ip')) -- 获取ip情况
	entry({"admin", "services", "bypass", "status"}, call("status")).leaf = true
	entry({"admin", "services", "bypass", "socks_status"}, call("socks_status")).leaf = true
	entry({"admin", "services", "bypass", "connect_status"}, call("connect_status")).leaf = true
	entry({"admin", "services", "bypass", "check_port"}, call("check_port")).leaf = true
end

function subscribe()
	luci.sys.call("/usr/share/bypass/subscribe >> /tmp/bypass.log 2>&1")
	luci.http.prepare_content("application/json")
	luci.http.write_json({ret=1})
end

function act_status()
	local e={}
	e.running=luci.sys.call("ps -w | grep by-retcp | grep -v grep >/dev/null")==0
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end

function act_ping()
	local e = {}
	local domain = luci.http.formvalue("domain")
	local port = luci.http.formvalue("port")
	local transport = luci.http.formvalue("transport")
	local wsPath = luci.http.formvalue("wsPath")
	local tls = luci.http.formvalue("tls")
	e.index = luci.http.formvalue("index")
	local iret = luci.sys.call("ipset add ss_spec_wan_ac " .. domain .. " 2>/dev/null")
	if transport == "ws" then
		local prefix = tls=='1' and "https://" or "http://"
		local address = prefix..domain..':'..port..wsPath
		local result = luci.sys.exec("curl --http1.1 -m 3 -s  -i -N -o /dev/null -w 'time_connect=%{time_connect}\nhttp_code=%{http_code}' -H 'Connection: Upgrade' -H 'Upgrade: websocket' -H 'Sec-WebSocket-Key: SGVsbG8sIHdvcmxkIQ==' -H 'Sec-WebSocket-Version: 13' "..address)
		e.socket = string.match(result,"http_code=(%d+)")=="101"
		e.ping = tonumber(string.match(result, "time_connect=(%d+.%d%d%d)"))*1000
	else
		local socket = nixio.socket("inet", "stream")
		socket:setopt("socket", "rcvtimeo", 3)
		socket:setopt("socket", "sndtimeo", 3)
		e.socket = socket:connect(domain, port)
		socket:close()
		-- 	e.ping = luci.sys.exec("ping -c 1 -W 1 %q 2>&1 | grep -o 'time=[0-9]*.[0-9]' | awk -F '=' '{print$2}'" % domain)
		-- 	if (e.ping == "") then
		e.ping = luci.sys.exec(string.format("echo -n $(tcping -q -c 1 -i 1 -t 2 -p %s %s 2>&1 | grep -o 'time=[0-9]*' | awk -F '=' '{print $2}') 2>/dev/null", port, domain))
		-- 	end
												
	end
	if (iret == 0) then
		luci.sys.call(" ipset del ss_spec_wan_ac " .. domain)
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end

function check_status()
	sret=luci.sys.call("curl -so /dev/null -m 3 www."..luci.http.formvalue("set")..".com")
	if sret==0 then
		retstring="0"
	else
		retstring="1"
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json({ret=retstring})
end

function refresh_data()
	local set=luci.http.formvalue("set")
	local icount=0
	if set=="gfw_data" then
		sret=luci.sys.call("curl -Lfso /tmp/gfw.b64 https://cdn.jsdelivr.net/gh/gfwlist/gfwlist/gfwlist.txt")
		if sret==0 then
			luci.sys.call("/usr/share/bypass/by-gfw")
			icount=luci.sys.exec("cat /tmp/gfwnew.txt | wc -l")
			if tonumber(icount)>1000 then
				oldcount=luci.sys.exec("cat /tmp/bypass/gfw.list | wc -l")
				if tonumber(icount)~=tonumber(oldcount) then
					luci.sys.exec("cp -f /tmp/gfwnew.txt /tmp/bypass/gfw.list && /etc/init.d/bypass restart >/dev/null 2>&1")
					retstring=tostring(tonumber(icount))
				else
					retstring="0"
				end
			else
				retstring="-1"
			end
			luci.sys.exec("rm -f /tmp/gfwnew.txt ")
		else
			retstring="-1"
		end
	elseif set=="ip_data" then
		sret=luci.sys.call("A=`curl -Lfsm 9 https://cdn.jsdelivr.net/gh/f6UiINtMDSmglMK4/A9xehMB2/ht2ix0v4Aj/zp2XmWPY9R4 || curl -Lfsm 9 https://raw.githubusercontent.com/f6UiINtMDSmglMK4/A9xehMB2/master/ht2ix0v4Aj/zp2XmWPY9R4` && echo \"$A\" | base64 -d > /tmp/china.txt")
		icount=luci.sys.exec("cat /tmp/china.txt | wc -l")
		if sret==0 and tonumber(icount)>1000 then
			oldcount=luci.sys.exec("cat /tmp/bypass/china.txt | wc -l")
			if tonumber(icount)~=tonumber(oldcount) then
				luci.sys.exec("cp -f /tmp/china.txt /tmp/bypass/china.txt && ipset list china_v4 >/dev/null 2>&1 && /usr/share/bypass/chinaipset")
				retstring=tostring(tonumber(icount))
			else
				retstring="0"
			end
		else
			retstring="-1"
		end
		luci.sys.exec("rm -f /tmp/china.txt ")
	elseif set=="ip6_data" then
		sret=luci.sys.call("curl -Lfso /tmp/china_v6.txt https://cdn.jsdelivr.net/gh/icy37785/Auto_IP_Range/China_ipv6.txt || curl -Lfso /tmp/china_v6.txt https://raw.githubusercontent.com/icy37785/Auto_IP_Range/master/China_ipv6.txt")
		icount=luci.sys.exec("cat /tmp/china_v6.txt | wc -l")
		if sret==0 and tonumber(icount)>1000 then
			oldcount=luci.sys.exec("cat /tmp/bypass/china_v6.txt | wc -l")
			if tonumber(icount)~=tonumber(oldcount) then
				luci.sys.exec("cp -f /tmp/china_v6.txt /tmp/bypass/china_v6.txt && ipset list china_v6 >/dev/null 2>&1 && /usr/share/bypass/chinaipset v6")
				retstring=tostring(tonumber(icount))
			else
				retstring="0"
			end
		else
			retstring="-1"
		end
		luci.sys.exec("rm -f /tmp/china_v6.txt ")
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json({ret=retstring,retcount=icount})
end

function check_port()
	local retstring="<br/>"
	local s
	local server_name
	local iret=1
	luci.model.uci.cursor():foreach("bypass","servers",function(s)
		if s.alias then
			server_name=s.alias
		elseif s.server and s.server_port then
			server_name="%s:%s"%{s.server,s.server_port}
		end
		luci.sys.exec(s.server..">>/a")
		local dp=luci.sys.exec("netstat -unl | grep 5336 >/dev/null && echo -n 5336 || echo -n 53")
		local ip=luci.sys.exec("echo "..s.server.." | grep -E \"^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$\" || \\\
		nslookup "..s.server.." 127.0.0.1#"..dp.." 2>/dev/null | grep Address | awk -F' ' '{print$NF}' | grep -E \"^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$\" | sed -n 1p")
		ip=luci.sys.exec("echo -n "..ip)
		iret=luci.sys.call("ipset add ss_spec_wan_ac "..ip.." 2>/dev/null")
		socket=nixio.socket("inet","stream")
		socket:setopt("socket","rcvtimeo",3)
		socket:setopt("socket","sndtimeo",3)
		ret=socket:connect(ip,s.server_port)
		socket:close()
		if tostring(ret)=="true" then
			retstring=retstring.."<font color='green'>["..server_name.."] OK.</font><br/>"
		else
			retstring=retstring.."<font color='red'>["..server_name.."] Error.</font><br/>"
		end
		if  iret==0 then
			luci.sys.call("ipset del ss_spec_wan_ac "..ip)
		end
	end)
	luci.http.prepare_content("application/json")
	luci.http.write_json({ret=retstring})
end

local function http_write_json(content)
	http.prepare_content("application/json")
	http.write_json(content or {code = 1})
end

function xray_check()
	local json = xray.to_check("")
	http_write_json(json)
end

function xray_update()
	local json = nil
	local task = http.formvalue("task")
	if task == "extract" then
		json = xray.to_extract(http.formvalue("file"), http.formvalue("subfix"))
	elseif task == "move" then
		json = xray.to_move(http.formvalue("file"))
	else
		json = xray.to_download(http.formvalue("url"))
	end

	http_write_json(json)
end

function trojan_go_check()
	local json = trojan_go.to_check("")
	http_write_json(json)
end

function trojan_go_update()
	local json = nil
	local task = http.formvalue("task")
	if task == "extract" then
		json = trojan_go.to_extract(http.formvalue("file"), http.formvalue("subfix"))
	elseif task == "move" then
		json = trojan_go.to_move(http.formvalue("file"))
	else
		json = trojan_go.to_download(http.formvalue("url"))
	end

	http_write_json(json)
end

function get_iso(ip)
    local mm = require 'maxminddb'
    local db = mm.open('/usr/share/bypass/GeoLite2-Country.mmdb')
    local res = db:lookup(ip)
    return string.lower(res:get('country', 'iso_code'))
end

function get_cname(ip)
    local mm = require 'maxminddb'
    local db = mm.open('/usr/share/bypass/GeoLite2-Country.mmdb')
    local res = db:lookup(ip)
    return string.lower(res:get('country', 'names', 'zh-CN'))
end

function check_site(host, port)
    local nixio = require "nixio"
    local socket = nixio.socket("inet", "stream")
    socket:setopt("socket", "rcvtimeo", 2)
    socket:setopt("socket", "sndtimeo", 2)
    local ret = socket:connect(host, port)
    socket:close()
    return ret
end

-- 获取当前代理状态 与节点ip
function check_ip()
    local e = {}
    local d = {}
    local port = 80
    local ip = luci.sys.exec('curl --retry 3 -m 10 -LfsA "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/86.0.4240.183 Safari/537.36" http://api.ipify.org/')
    d.flag = 'un'
    d.country = 'Unknown'
    if (ip ~= '') then
        local status, code = pcall(get_iso, ip)
        if (status) then
            d.flag = code
        end
        local status1, country = pcall(get_cname, ip)
        if (status1) then
            d.country = country
        end
    end
    e.outboard = ip
    e.outboardip = d
    e.baidu = check_site('www.baidu.com', port)
    e.taobao = check_site('www.taobao.com', port)
    e.google = check_site('www.google.com', port)
    e.youtube = check_site('www.youtube.com', port)
    luci.http.prepare_content('application/json')
    luci.http.write_json(e)
end

function status()
	local e = {}
	e.dns_mode_status = luci.sys.call("pidof smartdns-le >/dev/null") == 0
	e.socks5_status = luci.sys.call("ps -w | grep by- | grep socks5 | grep -v grep >/dev/null") == 0
	e.tcp_node_status = luci.sys.call("ps -w | grep by-retcp | grep -v grep >/dev/null") == 0
	e.udp_node_status = luci.sys.call("ps -w | grep by-reudp | grep -v grep >/dev/null") == 0
	e.kcptun_tcp_node_status = luci.sys.call("pidof kcptun-client >/dev/null") == 0
	e.nf_node_status = luci.sys.call("ps -w | grep by-nf | grep -v grep >/dev/null") == 0
	e.server_status = luci.sys.call("ps -w | grep by-server | grep -v grep >/dev/null") == 0
	e.chinadns_status = luci.sys.call("ps -w | grep chinadns-ng | grep -v grep >/dev/null") == 0
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end

function connect_status()
	local e = {}
	e.use_time = ""
	local url = luci.http.formvalue("url")
	local result = luci.sys.exec('curl --connect-timeout 3 -o /dev/null -I -skL -w "%{http_code}:%{time_starttransfer}" ' .. url)
	local code = tonumber(luci.sys.exec("echo -n '" .. result .. "' | awk -F ':' '{print $1}'") or "0")
	if code ~= 0 then
		local use_time = luci.sys.exec("echo -n '" .. result .. "' | awk -F ':' '{print $2}'")
		if use_time:find("%.") then
			e.use_time = string.format("%.2f", use_time * 1000)
		else
			e.use_time = string.format("%.2f", use_time / 1000)
		end
		e.ping_type = "curl"
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end
