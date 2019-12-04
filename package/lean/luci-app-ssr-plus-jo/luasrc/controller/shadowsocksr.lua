-- Copyright (C) 2018 jerrykuku <jerrykuku@qq.com>
-- Licensed to the public under the GNU General Public License v3.
module("luci.controller.shadowsocksr", package.seeall)

function index()
    if not nixio.fs.access("/etc/config/shadowsocksr") then
        return
    end
    if nixio.fs.access("/usr/bin/ssr-redir") then
        entry({"admin", "services", "shadowsocksr"},alias("admin", "services", "shadowsocksr", "client"), _("ShadowSocksR Plus+"),10).dependent = true
        entry({"admin", "services", "shadowsocksr", "client"},cbi("shadowsocksr/client"),_("SSR Client"),10).leaf = true
        entry({"admin", "services", "shadowsocksr", "servers"}, cbi("shadowsocksr/servers"), _("Severs Nodes"), 11).leaf = true
        entry({"admin", "services", "shadowsocksr", "servers"},arcombine(cbi("shadowsocksr/servers"), cbi("shadowsocksr/client-config")),_("Severs Nodes"), 11).leaf = true

 entry({"admin", "services", "shadowsocksr", "subscription"},cbi("shadowsocksr/subscription"), _("Subscription Managenent"),12).leaf = true
        entry({"admin", "services", "shadowsocksr", "control"},cbi("shadowsocksr/control"),_("Access Control"),13).leaf = true

        entry({"admin", "services", "shadowsocksr", "list"},cbi("shadowsocksr/list"),_("GFW List"),15).leaf = true

          
 entry({"admin", "services", "shadowsocksr", "automatic"},cbi("shadowsocksr/automatic"), _("Automatic Switching"),20).leaf = true
        entry({"admin", "services", "shadowsocksr", "advanced"},cbi("shadowsocksr/advanced"), _("Advanced Settings"),21).leaf = true
    elseif nixio.fs.access("/usr/bin/ssr-server") then
        entry({"admin", "services", "shadowsocksr"},alias("admin", "services", "shadowsocksr", "server"), _("ShadowSocksR"),10).dependent = true
    else
        return
    end
    if nixio.fs.access("/usr/bin/ssr-server") then
        entry({"admin", "services", "shadowsocksr", "server"},arcombine(cbi("shadowsocksr/server"), cbi("shadowsocksr/server-config")),_("SSR Server"),22).leaf = true
    end
    entry({"admin", "services", "shadowsocksr", "status"},form("shadowsocksr/status"),_("Status"), 23).leaf = true
    entry({"admin", "services", "shadowsocksr", "log"}, cbi("shadowsocksr/log"), _("Log"), 30).leaf = true

    entry({"admin", "services", "shadowsocksr", "refresh"}, call("refresh_data"))
    entry({"admin", "services", "shadowsocksr", "checkport"}, call("check_port"))
    entry({"admin", "services", "shadowsocksr", "checkports"}, call("check_ports"))
    entry({"admin", "services", "shadowsocksr", "run"}, call("act_status"))
    entry({"admin", "services", "shadowsocksr", "change"}, call("change_node"))
    entry({"admin", "services", "shadowsocksr", "allserver"}, call("get_servers"))
    entry({"admin", "services", "shadowsocksr", "ping"}, call("act_ping")).leaf=true
end


-- 检测全局服务器状态
function act_status()
    local e={}
    --全局服务器
    e.global=luci.sys.call("ps -w | grep ssr-retcp | grep -v grep >/dev/null") == 0  

     --检测PDNSD状态
	if tonumber(luci.sys.exec("ps -w | grep pdnsd |grep -v grep| wc -l"))>0 then
		e.pdnsd= true
	elseif tonumber(luci.sys.exec("ps -w | grep dnsparsing |grep -v grep| wc -l"))>0 then
		e.pdnsd= true
	elseif tonumber(luci.sys.exec("ps -w | grep dnscrypt-proxy |grep -v grep| wc -l"))>0 then
		e.pdnsd= true
              elseif tonumber(luci.sys.exec("ps -w | grep dns-forwarder |grep -v grep| wc -l"))>0 then
		e.pdnsd= true
               elseif tonumber(luci.sys.exec("ps -w | grep dnsforwarder |grep -v grep| wc -l"))>0 then
		e.pdnsd= true
                elseif tonumber(luci.sys.exec("ps -w | grep https_dns_proxy |grep -v grep| wc -l"))>0 then
		e.pdnsd= true
          end
     --检测SOCKS5状态
	if tonumber(luci.sys.exec("ps -w | grep ssr-local |grep -v grep| wc -l"))>0 then
		e.SOCKS5= true
	elseif tonumber(luci.sys.exec("ps -w | grep ss-local |grep -v grep| wc -l"))>0 then
		e.SOCKS5= true
	elseif tonumber(luci.sys.exec("ps -w | grep v2-ssr-local |grep -v grep| wc -l"))>0 then
		e.SOCKS5= true
	end    

--检测UDP2RAW状态
    if tonumber(luci.sys.exec("ps -w | grep udp2raw |grep -v grep| wc -l"))>0 then
		e.udp2raw= true  
end
--检测UDPspeeder状态
    if tonumber(luci.sys.exec("ps -w | grep udpspeeder |grep -v grep| wc -l"))>0 then
		e.udpspeeder= true  
end
 --检测服务端状态
	if tonumber(luci.sys.exec("ps -w | grep ssr-server |grep -v grep| wc -l"))>0 then
		e.server= true
end 
if luci.sys.call("pidof ssr-server >/dev/null") == 0 then
                   e.ssr_server= true
end 
	if luci.sys.call("pidof ss-server >/dev/null") == 0 then
		e.ss_server= true

end 
	if luci.sys.call("ps -w | grep v2ray-server | grep -v grep >/dev/null") == 0 then
		e.v2_server= true


	end       
    -- 检测游戏模式状态
    e.game = false
    if tonumber(luci.sys.exec("ps -w | grep ssr-reudp |grep -v grep| wc -l"))>0 then
        e.game= true
    else
        if tonumber(luci.sys.exec("ps -w | grep ssr-retcp |grep \"\\-u\"|grep -v grep| wc -l"))>0 then
            e.game= true
        end
    end
    -- 检测国内通道
    e.baidu = false
    sret = luci.sys.call("/usr/bin/ssr-check www.baidu.com 80 3 1")
    if sret == 0 then
        e.baidu =  true
    end
    
    -- 检测国外通道
    e.google = false
    sret = luci.sys.call("/usr/bin/ssr-check www.google.com 80 3 1")
    if sret == 0 then
        e.google =  true
    end
    
    luci.http.prepare_content("application/json")
    luci.http.write_json(e)
end

function act_ping()
	local e={}
	e.index=luci.http.formvalue("index")
	e.ping=luci.sys.exec("ping -c 1 -W 1 %q 2>&1 | grep -o 'time=[0-9]*.[0-9]' | awk -F '=' '{print$2}'"%luci.http.formvalue("domain"))
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end

function check_status()
	local set ="/usr/bin/ssr-check www." .. luci.http.formvalue("set") .. ".com 80 3 1"
	sret=luci.sys.call(set)
	if sret== 0 then
		retstring ="0"
	else
		retstring ="1"
	end	

	luci.http.prepare_content("application/json")
	luci.http.write_json({ ret=retstring })
end


function refresh_data()
    local set = luci.http.formvalue("set")
    local icount = 0

    if set == "gfw_data" then
        if nixio.fs.access("/usr/bin/wget-ssl") then
            refresh_cmd =
                "wget-ssl --no-check-certificate https://raw.githubusercontent.com/gfwlist/gfwlist/master/gfwlist.txt -O /tmp/gfw.b64"
        else
            refresh_cmd = "wget -O /tmp/gfw.b64 http://iytc.net/tools/list.b64"
        end
        sret = luci.sys.call(refresh_cmd .. " 2>/dev/null")
        if sret == 0 then
            luci.sys.call("/usr/bin/ssr-gfw")
            icount = luci.sys.exec("cat /tmp/gfwnew.txt | wc -l")
            if tonumber(icount) > 1000 then
                oldcount = luci.sys.exec("cat /etc/dnsmasq.ssr/gfw_list.conf | wc -l")
                if tonumber(icount) ~= tonumber(oldcount) then
                    luci.sys.exec("cp -f /tmp/gfwnew.txt /etc/dnsmasq.ssr/gfw_list.conf")
                    retstring = tostring(math.ceil(tonumber(icount) / 2))
                else
                    retstring = "0"
                end
            else
                retstring = "-1"
            end
            luci.sys.exec("rm -f /tmp/gfwnew.txt ")
        else
            retstring = "-1"
        end
    elseif set == "ip_data" then
        refresh_cmd =
            'wget -O- \'http://ftp.apnic.net/apnic/stats/apnic/delegated-apnic-latest\'  2>/dev/null| awk -F\\| \'/CN\\|ipv4/ { printf("%s/%d\\n", $4, 32-log($5)/log(2)) }\' > /tmp/china_ssr.txt'
        sret = luci.sys.call(refresh_cmd)
        icount = luci.sys.exec("cat /tmp/china_ssr.txt | wc -l")
        if sret == 0 and tonumber(icount) > 1000 then
            oldcount = luci.sys.exec("cat /etc/china_ssr.txt | wc -l")
            if tonumber(icount) ~= tonumber(oldcount) then
                luci.sys.exec("cp -f /tmp/china_ssr.txt /etc/china_ssr.txt")
                retstring = tostring(tonumber(icount))
            else
                retstring = "0"
            end
        else
            retstring = "-1"
        end
        luci.sys.exec("rm -f /tmp/china_ssr.txt ")
    else
        if nixio.fs.access("/usr/bin/wget-ssl") then
            refresh_cmd =
                "wget --no-check-certificate -O - https://easylist-downloads.adblockplus.org/easylistchina+easylist.txt | grep ^\\|\\|[^\\*]*\\^$ | sed -e 's:||:address\\=\\/:' -e 's:\\^:/127\\.0\\.0\\.1:' > /tmp/ad.conf"
        else
            refresh_cmd = "wget -O /tmp/ad.conf http://iytc.net/tools/ad.conf"
        end
        sret = luci.sys.call(refresh_cmd .. " 2>/dev/null")
        if sret == 0 then
            icount = luci.sys.exec("cat /tmp/ad.conf | wc -l")
            if tonumber(icount) > 1000 then
                if nixio.fs.access("/etc/dnsmasq.ssr/ad.conf") then
                    oldcount = luci.sys.exec("cat /etc/dnsmasq.ssr/ad.conf | wc -l")
                else
                    oldcount = 0
                end

                if tonumber(icount) ~= tonumber(oldcount) then
                    luci.sys.exec("cp -f /tmp/ad.conf /etc/dnsmasq.ssr/ad.conf")
                    retstring = tostring(math.ceil(tonumber(icount)))
                    if oldcount == 0 then
                        luci.sys.call("/etc/init.d/dnsmasq restart")
                    end
                else
                    retstring = "0"
                end
            else
                retstring = "-1"
            end
            luci.sys.exec("rm -f /tmp/ad.conf ")
        else
            retstring = "-1"
        end
    end
    luci.http.prepare_content("application/json")
    luci.http.write_json({ret = retstring, retcount = icount})
end


function act_ping()
	local e={}
	e.index=luci.http.formvalue("index")
	e.ping=luci.sys.exec("ping -c 1 -W 1 %q 2>&1 | grep -o 'time=[0-9]*.[0-9]' | awk -F '=' '{print$2}'"%luci.http.formvalue("domain"))
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end

function check_ports()
    local set = ""
    local retstring = "<br /><br />"
    local s
    local server_name = ""
    local shadowsocksr = "shadowsocksr"
    local uci = luci.model.uci.cursor()
    local iret = 1

    uci:foreach(
        shadowsocksr,
        "servers",
        function(s)
            if s.alias then
                server_name = s.alias
            elseif s.server and s.server_port then
                server_name = "%s:%s" % {s.server, s.server_port}
            end
            iret = luci.sys.call(" ipset add ss_spec_wan_ac " .. s.server .. " 2>/dev/null")
            socket = nixio.socket("inet", "stream")
            socket:setopt("socket", "rcvtimeo", 3)
            socket:setopt("socket", "sndtimeo", 3)
            ret = socket:connect(s.server, s.server_port)
            if tostring(ret) == "true" then
                socket:close()
                retstring = retstring .. "<font color='green'>[" .. server_name .. "] OK.</font><br />"
            else
                retstring = retstring .. "<font color='red'>[" .. server_name .. "] Error.</font><br />"
            end
            if iret == 0 then
                luci.sys.call(" ipset del ss_spec_wan_ac " .. s.server)
            end
        end
    )

    luci.http.prepare_content("application/json")
    luci.http.write_json({ret = retstring})
end


