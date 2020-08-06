-- Copyright 2018-2020 Lienol <lawlienol@gmail.com>
module("luci.controller.passwall", package.seeall)
local appname = "passwall"
local ucic = luci.model.uci.cursor()
local http = require "luci.http"
local kcptun = require "luci.model.cbi.passwall.api.kcptun"
local brook = require "luci.model.cbi.passwall.api.brook"
local v2ray = require "luci.model.cbi.passwall.api.v2ray"
local trojan_go = require "luci.model.cbi.passwall.api.trojan_go"

function index()
	appname = "passwall"
	entry({"admin", "services", appname}).dependent = true
	entry({"admin", "services", appname, "reset_config"}, call("reset_config")).leaf = true
	entry({"admin", "services", appname, "show"}, call("show_menu")).leaf = true
	entry({"admin", "services", appname, "hide"}, call("hide_menu")).leaf = true
	if not nixio.fs.access("/etc/config/passwall") then return end
	if nixio.fs.access("/etc/config/passwall_show") then
		entry({"admin", "services", appname}, alias("admin", "services", appname, "settings"), _("Pass Wall"), 1).dependent = true
	end
	entry({"admin", "services", appname, "settings"}, cbi("passwall/global"), _("Basic Settings"), 1).dependent = true
	entry({"admin", "services", appname, "node_list"}, cbi("passwall/node_list"), _("Node List"), 2).dependent = true
	entry({"admin", "services", appname, "auto_switch"}, cbi("passwall/auto_switch"), _("Auto Switch"), 3).leaf = true
	entry({"admin", "services", appname, "other"}, cbi("passwall/other", {autoapply = true}), _("Other Settings"), 93).leaf = true
	if nixio.fs.access("/usr/sbin/haproxy") then
		entry({"admin", "services", appname, "haproxy"}, cbi("passwall/haproxy"), _("Load Balancing"), 94).leaf = true
	end
	entry({"admin", "services", appname, "node_subscribe"}, cbi("passwall/node_subscribe"), _("Node Subscribe"), 95).dependent = true
	entry({"admin", "services", appname, "rule"}, cbi("passwall/rule"), _("Rule Manage"), 96).leaf = true
	entry({"admin", "services", appname, "app_update"}, cbi("passwall/app_update"), _("App Update"), 97).leaf = true
	entry({"admin", "services", appname, "node_config"}, cbi("passwall/node_config")).leaf = true
	entry({"admin", "services", appname, "shunt_rules"}, cbi("passwall/shunt_rules")).leaf = true
	entry({"admin", "services", appname, "acl"}, cbi("passwall/acl"), _("Access control"), 98).leaf = true
	entry({"admin", "services", appname, "log"}, form("passwall/log"), _("Watch Logs"), 999).leaf = true
	entry({"admin", "services", appname, "server"}, cbi("passwall/server/index"), _("Server-Side"), 99).leaf = true
	entry({"admin", "services", appname, "server_user"}, cbi("passwall/server/user")).leaf = true

	entry({"admin", "services", appname, "server_user_status"}, call("server_user_status")).leaf = true
	entry({"admin", "services", appname, "server_get_log"}, call("server_get_log")).leaf = true
	entry({"admin", "services", appname, "server_clear_log"}, call("server_clear_log")).leaf = true
	entry({"admin", "services", appname, "link_append_temp"}, call("link_append_temp")).leaf = true
	entry({"admin", "services", appname, "link_load_temp"}, call("link_load_temp")).leaf = true
	entry({"admin", "services", appname, "link_clear_temp"}, call("link_clear_temp")).leaf = true
	entry({"admin", "services", appname, "link_add_node"}, call("link_add_node")).leaf = true
	entry({"admin", "services", appname, "get_log"}, call("get_log")).leaf = true
	entry({"admin", "services", appname, "clear_log"}, call("clear_log")).leaf = true
	entry({"admin", "services", appname, "status"}, call("status")).leaf = true
	entry({"admin", "services", appname, "socks_status"}, call("socks_status")).leaf = true
	entry({"admin", "services", appname, "connect_status"}, call("connect_status")).leaf = true
	entry({"admin", "services", appname, "check_port"}, call("check_port")).leaf = true
	entry({"admin", "services", appname, "ping_node"}, call("ping_node")).leaf = true
	entry({"admin", "services", appname, "set_node"}, call("set_node")).leaf = true
	entry({"admin", "services", appname, "copy_node"}, call("copy_node")).leaf = true
	entry({"admin", "services", appname, "clear_all_nodes"}, call("clear_all_nodes")).leaf = true
	entry({"admin", "services", appname, "delete_select_nodes"}, call("delete_select_nodes")).leaf = true
	entry({"admin", "services", appname, "update_rules"}, call("update_rules")).leaf = true
	entry({"admin", "services", appname, "luci_check"}, call("luci_check")).leaf = true
	entry({"admin", "services", appname, "luci_update"}, call("luci_update")).leaf = true
	entry({"admin", "services", appname, "kcptun_check"}, call("kcptun_check")).leaf = true
	entry({"admin", "services", appname, "kcptun_update"}, call("kcptun_update")).leaf = true
	entry({"admin", "services", appname, "brook_check"}, call("brook_check")).leaf = true
	entry({"admin", "services", appname, "brook_update"}, call("brook_update")).leaf = true
	entry({"admin", "services", appname, "v2ray_check"}, call("v2ray_check")).leaf = true
	entry({"admin", "services", appname, "v2ray_update"}, call("v2ray_update")).leaf = true
	entry({"admin", "services", appname, "trojan_go_check"}, call("trojan_go_check")).leaf = true
	entry({"admin", "services", appname, "trojan_go_update"}, call("trojan_go_update")).leaf = true
end

local function http_write_json(content)
	http.prepare_content("application/json")
	http.write_json(content or {code = 1})
end

function reset_config()
	luci.sys.call('[ -f "/usr/share/passwall/config.default" ] && cp -f /usr/share/passwall/config.default /etc/config/passwall && /etc/init.d/passwall reload')
	luci.http.redirect(luci.dispatcher.build_url("admin", "services", appname))
end

function show_menu()
	luci.sys.call("touch /etc/config/passwall_show")
	luci.http.redirect(luci.dispatcher.build_url("admin", "services", appname))
end

function hide_menu()
	luci.sys.call("rm -rf /etc/config/passwall_show")
	luci.http.redirect(luci.dispatcher.build_url("admin", "status", "overview"))
end

function link_append_temp()
	local link = luci.http.formvalue("link")
	local lfile = "/tmp/links.conf"
	local ret, ldata="empty", {}
	luci.sys.call('touch ' .. lfile .. ' && echo \'' .. link .. '\' >> ' .. lfile)
	ret = luci.sys.exec([[awk -F'://' 'BEGIN{ all=0 } /.{2,9}:\/\/.{4,}$/ {gsub(/:\/\/.*$/,""); arr[$0]++; all++ } END { for(typ in arr) { printf("%s: %d, ", typ, arr[typ]) }; printf("\ntotal: %d", all) }' ]] .. lfile)
	luci.http.prepare_content("application/json")
	luci.http.write_json({counter = ret})
end

function link_load_temp()
	local lfile = "/tmp/links.conf"
	local ret, ldata="empty", {}
	ldata[#ldata+1] = nixio.fs.readfile(lfile) or "_nofile_"
	if ldata[1] == "" then
		ldata[1] = "_nodata_"
	else
		ret = luci.sys.exec([[awk -F'://' 'BEGIN{ all=0 } /.{2,9}:\/\/.{4,}$/ {gsub(/:\/\/.*$/,""); arr[$0]++; all++ } END { for(typ in arr) { printf("%s: %d, ", typ, arr[typ]) }; printf("\ntotal: %d", all) }' ]] .. lfile)
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json({counter = ret, data = ldata})
end

function link_clear_temp()
	local lfile = "/tmp/links.conf"
	luci.sys.call('cat /dev/null > ' .. lfile)
end

function link_add_node()
	local lfile = "/tmp/links.conf"
	local link = luci.http.formvalue("link")
	luci.sys.call('echo \'' .. link .. '\' >> ' .. lfile)
	luci.sys.call("lua /usr/share/passwall/subscribe.lua add log")
end

function get_log()
	-- luci.sys.exec("[ -f /var/log/passwall.log ] && sed '1!G;h;$!d' /var/log/passwall.log > /var/log/passwall_show.log")
	luci.http.write(luci.sys.exec("[ -f '/var/log/passwall.log' ] && cat /var/log/passwall.log"))
end

function clear_log()
	luci.sys.call("echo '' > /var/log/passwall.log")
end

function status()
	-- local dns_mode = ucic:get(appname, "@global[0]", "dns_mode")
	local e = {}
	e.dns_mode_status = luci.sys.call("netstat -apn | grep ':7913 ' >/dev/null") == 0
	e.haproxy_status = luci.sys.call(string.format("ps -w | grep -v grep | grep '%s/bin/' | grep haproxy >/dev/null", appname)) == 0
	local tcp_node_num = ucic:get(appname, "@global_other[0]", "tcp_node_num") or 1
	for i = 1, tcp_node_num, 1 do
		e["kcptun_tcp_node%s_status" % i] = luci.sys.call(string.format("ps -w | grep -v grep | grep '%s/bin/kcptun' | grep -i 'tcp_%s' >/dev/null", appname, i)) == 0
		e["tcp_node%s_status" % i] = luci.sys.call(string.format("ps -w | grep -v -E 'grep|kcptun' | grep '%s/bin/' | grep -i 'TCP_%s' >/dev/null", appname, i)) == 0
	end

	local udp_node_num = ucic:get(appname, "@global_other[0]", "udp_node_num") or 1
	for i = 1, udp_node_num, 1 do
		if (ucic:get(appname, "@global[0]", "udp_node" .. i) or "nil") == "tcp" then
			e["udp_node%s_status" % i] = e["tcp_node%s_status" % i]
		else
			e["udp_node%s_status" % i] = luci.sys.call(string.format("ps -w | grep -v grep | grep '%s/bin/' | grep -i 'UDP_%s' >/dev/null", appname, i)) == 0
		end
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end

function socks_status()
	local e = {}
	local index = luci.http.formvalue("index")
	local id = luci.http.formvalue("id")
	e.index = index
	e.status = luci.sys.call(string.format("ps -w | grep -v grep | grep '%s/bin/' | grep 'SOCKS_%s' > /dev/null", appname, id)) == 0
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end

function connect_status()
	local e = {}
	e.use_time = ""
	local url = luci.http.formvalue("url")
	local result = luci.sys.exec('curl --connect-timeout 5 -o /dev/null -I -skL -w "%{http_code}:%{time_starttransfer}" ' .. url)
	local code = tonumber(luci.sys.exec("echo -n '" .. result .. "' | awk -F ':' '{print $1}'") or "0")
	if code ~= 0 then
		local use_time = luci.sys.exec("echo -n '" .. result .. "' | awk -F ':' '{print $2}'")
		e.use_time = string.format("%.2f", use_time * 1000)
		e.ping_type = "curl"
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end

function ping_node()
	local index = luci.http.formvalue("index")
	local address = luci.http.formvalue("address")
	local port = luci.http.formvalue("port")
	local e = {}
	e.index = index
	if (ucic:get(appname, "@global_other[0]", "use_tcping") or 1)  == "1" and luci.sys.exec("echo -n $(command -v tcping)") ~= "" then
		e.ping = luci.sys.exec(string.format("echo -n $(tcping -q -c 1 -i 1 -t 2 -p %s %s 2>&1 | grep -o 'time=[0-9]*' | awk -F '=' '{print $2}') 2>/dev/null", port, address))
	end
	if e.ping == nil or tonumber(e.ping) == 0 then
		e.ping = luci.sys.exec("echo -n $(ping -c 1 -W 1 %q 2>&1 | grep -o 'time=[0-9]*' | awk -F '=' '{print $2}') 2>/dev/null" % address)
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end

function set_node()
	local protocol = luci.http.formvalue("protocol")
	local number = luci.http.formvalue("number")
	local section = luci.http.formvalue("section")
	ucic:set(appname, "@global[0]", protocol .. "_node" .. number, section)
	ucic:commit(appname)
	luci.sys.call("/etc/init.d/passwall restart > /dev/null 2>&1 &")
	luci.http.redirect(luci.dispatcher.build_url("admin", "services", appname, "log"))
end

function copy_node()
	local e = {}
	local section = luci.http.formvalue("section")
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end

function clear_all_nodes()
	ucic:foreach(appname, "nodes", function(node)
		ucic:delete(appname, node['.name'])
	end)
	
	local function clear(type)
		local node_num = ucic:get(appname, "@global_other[0]", type .. "_node_num") or 1
		for i = 1, node_num, 1 do
			local node = ucic:get(appname, "@global[0]", type .. "_node" .. i)
			if node then
				ucic:set(appname, '@global[0]', type .. "_node" .. i, "nil")
			end
		end
	end
	clear("tcp")
	clear("udp")

	ucic:commit(appname)
	luci.sys.call("/etc/init.d/" .. appname .. " restart")
end

function delete_select_nodes()
	local ids = luci.http.formvalue("ids")
	string.gsub(ids, '[^' .. "," .. ']+', function(w)
		ucic:delete(appname, w)
	end)
	ucic:commit(appname)
	luci.sys.call("/etc/init.d/" .. appname .. " restart")
end

function check_port()
	local node_name = ""

	local retstring = "<br />"
	-- retstring = retstring .. "<font color='red'>暂时不支持UDP检测</font><br />"

	retstring = retstring .. "<font color='green'>检测端口可用性</font><br />"
	ucic:foreach(appname, "nodes", function(s)
		local ret = ""
		local tcp_socket
		if (s.use_kcp and s.use_kcp == "1" and s.kcp_port) or
			(s.v2ray_transport and s.v2ray_transport == "mkcp" and s.port) then
		else
			local type = s.type
			if type and type ~= "V2ray_balancing" and type ~= "V2ray_shunt" and
				s.address and s.port and s.remarks then
				node_name = "%s：[%s] %s:%s" % {s.type, s.remarks, s.address, s.port}
				tcp_socket = nixio.socket("inet", "stream")
				tcp_socket:setopt("socket", "rcvtimeo", 3)
				tcp_socket:setopt("socket", "sndtimeo", 3)
				ret = tcp_socket:connect(s.address, s.port)
				if tostring(ret) == "true" then
					retstring = retstring .. "<font color='green'>" .. node_name .. "   OK.</font><br />"
				else
					retstring = retstring .. "<font color='red'>" .. node_name .. "   Error.</font><br />"
				end
				ret = ""
			end
		end
		if tcp_socket then tcp_socket:close() end
	end)
	luci.http.prepare_content("application/json")
	luci.http.write_json({ret = retstring})
end

function update_rules()
	local update = luci.http.formvalue("update")
	luci.sys.call("lua /usr/share/passwall/rule_update.lua log '" .. update .. "' > /dev/null 2>&1 &")
end

function server_user_status()
	local e = {}
	e.index = luci.http.formvalue("index")
	e.status = luci.sys.call(string.format("ps -w | grep -v 'grep' | grep '%s/bin/' | grep -i '%s' >/dev/null", appname .. "_server", luci.http.formvalue("id"))) == 0
	http_write_json(e)
end

function server_get_log()
	luci.http.write(luci.sys.exec("[ -f '/var/log/passwall_server.log' ] && cat /var/log/passwall_server.log"))
end

function server_clear_log()
	luci.sys.call("echo '' > /var/log/passwall_server.log")
end

function kcptun_check()
	local json = kcptun.to_check("")
	http_write_json(json)
end

function kcptun_update()
	local json = nil
	local task = http.formvalue("task")
	if task == "extract" then
		json = kcptun.to_extract(http.formvalue("file"), http.formvalue("subfix"))
	elseif task == "move" then
		json = kcptun.to_move(http.formvalue("file"))
	else
		json = kcptun.to_download(http.formvalue("url"))
	end

	http_write_json(json)
end

function brook_check()
	local json = brook.to_check("")
	http_write_json(json)
end

function brook_update()
	local json = nil
	local task = http.formvalue("task")
	if task == "move" then
		json = brook.to_move(http.formvalue("file"))
	else
		json = brook.to_download(http.formvalue("url"))
	end

	http_write_json(json)
end

function v2ray_check()
	local json = v2ray.to_check("")
	http_write_json(json)
end

function v2ray_update()
	local json = nil
	local task = http.formvalue("task")
	if task == "extract" then
		json = v2ray.to_extract(http.formvalue("file"), http.formvalue("subfix"))
	elseif task == "move" then
		json = v2ray.to_move(http.formvalue("file"))
	else
		json = v2ray.to_download(http.formvalue("url"))
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
