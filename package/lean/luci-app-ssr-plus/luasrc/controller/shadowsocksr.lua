-- Copyright (C) 2017 yushi studio <ywb94@qq.com>
-- Licensed to the public under the GNU General Public License v3.

module("luci.controller.shadowsocksr", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/shadowsocksr") then
		return
	end
	entry({"admin", "services", "shadowsocksr"}, alias("admin", "services", "shadowsocksr", "client"),_("ShadowSocksR Plus+"), 10).dependent = true
	entry({"admin", "services", "shadowsocksr", "client"}, cbi("shadowsocksr/client"),_("SSR Client"), 10).leaf = true
	entry({"admin", "services", "shadowsocksr", "servers"}, arcombine(cbi("shadowsocksr/servers", {autoapply = true}), cbi("shadowsocksr/client-config")),_("Severs Nodes"), 20).leaf = true
	entry({"admin", "services", "shadowsocksr", "control"},cbi("shadowsocksr/control"), _("Access Control"), 30).leaf = true
	entry({"admin", "services", "shadowsocksr", "advanced"},cbi("shadowsocksr/advanced"),_("Advanced Settings"), 50).leaf = true
	entry({"admin", "services", "shadowsocksr", "server"},arcombine(cbi("shadowsocksr/server"), cbi("shadowsocksr/server-config")),_("SSR Server"), 60).leaf = true
	entry({"admin", "services", "shadowsocksr", "status"},form("shadowsocksr/status"),_("Status"), 70).leaf = true
	entry({"admin", "services", "shadowsocksr", "check"}, call("check_status"))
	entry({"admin", "services", "shadowsocksr", "refresh"}, call("refresh_data"))
	entry({"admin", "services", "shadowsocksr", "subscribe"}, call("subscribe"))
	entry({"admin", "services", "shadowsocksr", "checkport"}, call("check_port"))
	entry({"admin", "services", "shadowsocksr", "log"},form("shadowsocksr/log"),_("Log"), 80).leaf = true
	entry({"admin", "services", "shadowsocksr","run"},call("act_status")).leaf = true
	entry({"admin", "services", "shadowsocksr", "ping"}, call("act_ping")).leaf = true
end

function subscribe()
	luci.sys.call("/usr/bin/lua /usr/share/shadowsocksr/subscribe.lua >> /tmp/ssrplus.log 2>&1")
	luci.http.prepare_content("application/json")
	luci.http.write_json({ret = 1})
end

function act_status()
	local e = {}
	e.running = luci.sys.call("busybox ps -w | grep ssr-retcp | grep -v grep >/dev/null") == 0
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end

function act_ping()
	local e = {}
	local domain = luci.http.formvalue("domain")
	local port = luci.http.formvalue("port")
	e.index = luci.http.formvalue("index")
	local iret = luci.sys.call(" ipset add ss_spec_wan_ac " .. domain .. " 2>/dev/null")
	local socket = nixio.socket("inet", "stream")
	socket:setopt("socket", "rcvtimeo", 3)
	socket:setopt("socket", "sndtimeo", 3)
	e.socket = socket:connect(domain, port)
	socket:close()
	e.ping = luci.sys.exec("ping -c 1 -W 1 %q 2>&1 | grep -o 'time=[0-9]*.[0-9]' | awk -F '=' '{print$2}'" % domain)
	if (e.ping == "") then
		e.ping = luci.sys.exec(string.format("echo -n $(tcping -q -c 1 -i 1 -t 2 -p %s %s 2>&1 | grep -o 'time=[0-9]*' | awk -F '=' '{print $2}') 2>/dev/null",port, domain))
	end
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
		retstring = "0"
	else
		retstring = "1"
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json({ret = retstring})
end

function refresh_data()
	local set = luci.http.formvalue("set")
	local uci = luci.model.uci.cursor()
	local icount = 0
	local retstring = 0
	local function update(url, file, type, file2)
		local Num = 1
		refresh_cmd = "wget-ssl --no-check-certificate -t 3 -T 10 -O- " .. url .. " > /tmp/ssr-update." .. type
		sret = luci.sys.call(refresh_cmd .. " 2>/dev/null")
		if sret == 0 then
			if type == "gfw_data" then
				luci.sys.call("/usr/bin/ssr-gfw " .. type)
				Num = 2
			end
			if type == "ad_data" then
				luci.sys.call("/usr/bin/ssr-ad " .. type)
			end
			local new_md5 = luci.sys.exec("echo -n $([ -f '/tmp/ssr-update." .. type .. "' ] && md5sum /tmp/ssr-update." .. type .. " | awk '{print $1}')")
			local old_md5 = luci.sys.exec("echo -n $([ -f '" .. file .. "' ] && md5sum " .. file .. " | awk '{print $1}')")
			if new_md5 == old_md5 then
				retstring = "0"
			else
				icount = luci.sys.exec("cat /tmp/ssr-update." .. type .. " | wc -l")
				luci.sys.exec("cp -f /tmp/ssr-update." .. type .. " " .. file)
				if file2 then luci.sys.exec("cp -f /tmp/ssr-update." .. type .. " " .. file2) end
				retstring = tostring(tonumber(icount)/Num)
				if type == "gfw_data" or type == "ad_data" then
					luci.sys.exec("/usr/share/shadowsocksr/gfw2ipset.sh gfw_data")
				else
					luci.sys.exec("/etc/init.d/shadowsocksr restart &")
				end
			end
		else
			retstring = "-1"
		end
		luci.sys.exec("rm -f /tmp/ssr-update." .. type)
	end
	if set == "gfw_data" then
		update(uci:get_first("shadowsocksr", "global", "gfwlist_url", "https://cdn.jsdelivr.net/gh/gfwlist/gfwlist/gfwlist.txt"), "/etc/ssr/gfw_list.conf", set, "/tmp/dnsmasq.ssr/gfw_list.conf")
	end
	if set == "ip_data" then
		update(uci:get_first("shadowsocksr", "global", "chnroute_url","https://ispip.clang.cn/all_cn.txt"), "/etc/ssr/china_ssr.txt", set)
	end
	if set == "ad_data" then
		update(uci:get_first("shadowsocksr", "global", "adblock_url","https://easylist-downloads.adblockplus.org/easylistchina+easylist.txt"), "/etc/ssr/ad.conf", set, "/tmp/dnsmasq.ssr/ad.conf")
	end
	if set == "nfip_data" then
		update(uci:get_first("shadowsocksr", "global", "nfip_url","https://raw.githubusercontent.com/QiuSimons/Netflix_IP/master/NF_only.txt"), "/etc/ssr/netflixip.list", set)
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json({ret = retstring,retcount = icount})
end

function check_port()
	local set = ""
	local retstring = "<br /><br />"
	local s
	local server_name = ""
	local uci = luci.model.uci.cursor()
	local iret = 1
	uci:foreach("shadowsocksr", "servers", function(s)
		if s.alias then
			server_name = s.alias
		elseif s.server and s.server_port then
			server_name = "%s:%s" %{s.server, s.server_port}
		end
		iret = luci.sys.call("ipset add ss_spec_wan_ac " .. s.server .. " 2>/dev/null")
		socket = nixio.socket("inet", "stream")
		socket:setopt("socket", "rcvtimeo", 3)
		socket:setopt("socket", "sndtimeo", 3)
		ret = socket:connect(s.server,s.server_port)
		if tostring(ret) == "true" then
			socket:close()
			retstring = retstring .. "<font color = 'green'>[" .. server_name .. "] OK.</font><br />"
		else
			retstring = retstring .. "<font color = 'red'>[" .. server_name .. "] Error.</font><br />"
		end
		if iret == 0 then
			luci.sys.call("ipset del ss_spec_wan_ac " .. s.server)
		end
	end)
	luci.http.prepare_content("application/json")
	luci.http.write_json({ret = retstring})
end
