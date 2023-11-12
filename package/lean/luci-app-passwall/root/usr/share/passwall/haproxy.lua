#!/usr/bin/lua

local api = require ("luci.passwall.api")
local appname = api.appname
local fs = api.fs
local jsonc = api.jsonc
local uci = api.uci
local sys = api.sys

local log = function(...)
	api.log(...)
end

function get_ip_port_from(str)
	local result_port = sys.exec("echo -n " .. str .. " | sed -n 's/^.*[:#]\\([0-9]*\\)$/\\1/p'")
	local result_ip = sys.exec(string.format("__host=%s;__varport=%s;", str, result_port) .. "echo -n ${__host%%${__varport:+[:#]${__varport}*}}")
	return result_ip, result_port
end

local new_port
local function get_new_port()
	if new_port then
		new_port = tonumber(sys.exec(string.format("echo -n $(/usr/share/%s/app.sh get_new_port %s tcp)", appname, new_port + 1)))
	else
		new_port = tonumber(sys.exec(string.format("echo -n $(/usr/share/%s/app.sh get_new_port auto tcp)", appname)))
	end
	return new_port
end

local var = api.get_args(arg)
local haproxy_path = var["-path"]
local haproxy_conf = var["-conf"]
local haproxy_dns = var["-dns"] or "119.29.29.29:53,223.5.5.5:53"

local cpu_thread = sys.exec('echo -n $(cat /proc/cpuinfo | grep "processor" | wc -l)') or "1"
local health_check_type = uci:get(appname, "@global_haproxy[0]", "health_check_type") or "tcp"
local health_check_inter = uci:get(appname, "@global_haproxy[0]", "health_check_inter") or "10"

log("HAPROXY 负载均衡...")
fs.mkdir(haproxy_path)
local haproxy_file = haproxy_path .. "/" .. haproxy_conf

local f_out = io.open(haproxy_file, "a")

local haproxy_config = [[
global
	daemon
	log         127.0.0.1 local2
	maxconn     60000
	stats socket  {{path}}/haproxy.sock
	nbthread {{nbthread}}
	external-check
	insecure-fork-wanted

defaults
	mode                    tcp
	log                     global
	option                  tcplog
	option                  dontlognull
	option http-server-close
	#option forwardfor       except 127.0.0.0/8
	option                  redispatch
	retries                 2
	timeout http-request    10s
	timeout queue           1m
	timeout connect         10s
	timeout client          1m
	timeout server          1m
	timeout http-keep-alive 10s
	timeout check           10s
	maxconn                 3000
	
resolvers mydns
	resolve_retries       1
	timeout resolve       5s
	hold valid           600s
{{dns}}
]]

haproxy_config = haproxy_config:gsub("{{path}}",  haproxy_path)
haproxy_config = haproxy_config:gsub("{{nbthread}}",  cpu_thread)

local mydns = ""
local index = 0
string.gsub(haproxy_dns, '[^' .. "," .. ']+', function(w)
	index = index + 1
	local s = w:gsub("#", ":")
	if not s:find(":") then
		s = s .. ":53"
	end
	mydns = mydns .. (index > 1 and "\n" or "") .. "    " .. string.format("nameserver dns%s %s", index, s)
end)
haproxy_config = haproxy_config:gsub("{{dns}}",  mydns)

f_out:write(haproxy_config)

local listens = {}

uci:foreach(appname, "haproxy_config", function(t)
	if t.enabled == "1" then
		local server_remark
		local server_address
		local server_port
		local lbss = t.lbss
		local listen_port = tonumber(t.haproxy_port) or 0
		local server_node = uci:get_all(appname, lbss)
		if server_node and server_node.address and server_node.port then
			server_remark = server_node.address .. ":" .. server_node.port
			server_address = server_node.address
			server_port = server_node.port
			t.origin_address = server_address
			t.origin_port = server_port
			if health_check_type == "passwall_logic" then
				if server_node.type ~= "Socks" then
					local relay_port = server_node.port
					new_port = get_new_port()
					local config_file = string.format("haproxy_%s_%s.json", t[".name"], new_port)
					sys.call(string.format('/usr/share/%s/app.sh run_socks "%s"> /dev/null',
						appname,
						string.format("flag=%s node=%s bind=%s socks_port=%s config_file=%s",
							new_port, --flag
							server_node[".name"], --node
							"127.0.0.1", --bind
							new_port, --socks port
							config_file --config file
							)
						)
					)
					server_address = "127.0.0.1"
					server_port = new_port
				end
			end
		else
			server_address, server_port = get_ip_port_from(lbss)
			server_remark = server_address .. ":" .. server_port
			t.origin_address = server_address
			t.origin_port = server_port
		end
		if server_address and server_port and listen_port > 0 then
			if not listens[listen_port] then
				listens[listen_port] = {}
			end
			t.server_remark = server_remark
			t.server_address = server_address
			t.server_port = server_port
			table.insert(listens[listen_port], t)
		else
			log("  - 丢弃1个明显无效的节点")
		end
	end
end)

local sortTable = {}
for i in pairs(listens) do
	if i ~= nil then
		table.insert(sortTable, i)
	end
end
table.sort(sortTable, function(a,b) return (a < b) end)

for i, port in pairs(sortTable) do
	log("  + 入口 0.0.0.0:%s..." % port)

	f_out:write("\n" .. string.format([[
listen %s
	bind 0.0.0.0:%s
	mode tcp
	balance roundrobin
]], port, port))

	if health_check_type == "passwall_logic" then
		f_out:write(string.format([[
	option external-check
	external-check command "/usr/share/passwall/haproxy_check.sh"
]], port, port))
	end

	for i, o in ipairs(listens[port]) do
		local remark = o.server_remark
		local server = o.server_address .. ":" .. o.server_port
		local server_conf = "server {{remark}} {{server}} weight {{weight}} {{resolvers}} check inter {{inter}} rise 1 fall 3 {{backup}}"
		server_conf = server_conf:gsub("{{remark}}", remark)
		server_conf = server_conf:gsub("{{server}}", server)
		server_conf = server_conf:gsub("{{weight}}",  o.lbweight)
		local resolvers = "resolvers mydns"
		if api.is_ip(o.server_address) then
			resolvers = ""
		end
		server_conf = server_conf:gsub("{{resolvers}}",  resolvers)
		server_conf = server_conf:gsub("{{inter}}",  tonumber(health_check_inter) .. "s")
		server_conf = server_conf:gsub("{{backup}}",  o.backup == "1" and "backup" or "")

		f_out:write("    " .. server_conf .. "\n")

		if o.export ~= "0" then
			sys.call(string.format("/usr/share/passwall/app.sh add_ip2route %s %s", o.origin_address, o.export))
		end

		log(string.format("  | - 出口节点：%s:%s，权重：%s", o.origin_address, o.origin_port, o.lbweight))
	end
end

--控制台配置
local console_port = uci:get(appname, "@global_haproxy[0]", "console_port")
local console_user = uci:get(appname, "@global_haproxy[0]", "console_user")
local console_password = uci:get(appname, "@global_haproxy[0]", "console_password")
local str = [[
listen console
	bind 0.0.0.0:%s
	mode http
	stats refresh 30s
	stats uri /
	stats admin if TRUE
	%s
]]
f_out:write("\n" .. string.format(str, console_port, (console_user and console_user ~= "" and console_password and console_password ~= "") and "stats auth " .. console_user .. ":" .. console_password or ""))
log(string.format("  * 控制台端口：%s", console_port))

f_out:close()
