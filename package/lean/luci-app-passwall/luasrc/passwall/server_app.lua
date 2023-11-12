#!/usr/bin/lua

local action = arg[1]
local api = require "luci.passwall.api"
local sys = api.sys
local uci = api.uci
local jsonc = api.jsonc

local CONFIG = "passwall_server"
local CONFIG_PATH = "/tmp/etc/" .. CONFIG
local NFT_INCLUDE_FILE = CONFIG_PATH .. "/" .. CONFIG .. ".nft"
local LOG_APP_FILE = "/tmp/log/" .. CONFIG .. ".log"
local TMP_BIN_PATH = CONFIG_PATH .. "/bin"
local require_dir = "luci.passwall."

local ipt_bin = sys.exec("echo -n $(/usr/share/passwall/iptables.sh get_ipt_bin)")
local ip6t_bin = sys.exec("echo -n $(/usr/share/passwall/iptables.sh get_ip6t_bin)")

local nft_flag = api.is_finded("fw4") and "1" or "0"

local function log(...)
	local f, err = io.open(LOG_APP_FILE, "a")
	if f and err == nil then
		local str = os.date("%Y-%m-%d %H:%M:%S: ") .. table.concat({...}, " ")
		f:write(str .. "\n")
		f:close()
	end
end

local function cmd(cmd)
	sys.call(cmd)
end

local function ipt(arg)
	cmd(ipt_bin .. " -w " .. arg)
end

local function ip6t(arg)
	cmd(ip6t_bin .. " -w " .. arg)
end

local function ln_run(s, d, command, output)
	if not output then
		output = "/dev/null"
	end
	d = TMP_BIN_PATH .. "/" .. d
	cmd(string.format('[ ! -f "%s" ] && ln -s %s %s 2>/dev/null', d, s, d))
	return string.format("%s >%s 2>&1 &", d .. " " .. command, output)
end

local function gen_include()
	cmd(string.format("echo '#!/bin/sh' > /tmp/etc/%s.include", CONFIG))
	local function extract_rules(n, a)
		local _ipt = ipt_bin
		if n == "6" then
			_ipt = ip6t_bin
		end
		local result = "*" .. a
		result = result .. "\n" .. sys.exec(_ipt .. '-save -t ' .. a .. ' | grep "PSW-SERVER" | sed -e "s/^-A \\(INPUT\\)/-I \\1 1/"')
		result = result .. "COMMIT"
		return result
	end
	local f, err = io.open("/tmp/etc/" .. CONFIG .. ".include", "a")
	if f and err == nil then
		if nft_flag == "0" then
			f:write(ipt_bin .. '-save -c | grep -v "PSW-SERVER" | ' .. ipt_bin .. '-restore -c' .. "\n")
			f:write(ipt_bin .. '-restore -n <<-EOT' .. "\n")
			f:write(extract_rules("4", "filter") .. "\n")
			f:write("EOT" .. "\n")
			f:write(ip6t_bin .. '-save -c | grep -v "PSW-SERVER" | ' .. ip6t_bin .. '-restore -c' .. "\n")
			f:write(ip6t_bin .. '-restore -n <<-EOT' .. "\n")
			f:write(extract_rules("6", "filter") .. "\n")
			f:write("EOT" .. "\n")
			f:close()
		else
			f:write("nft -f " .. NFT_INCLUDE_FILE .. "\n")
			f:close()
		end
	end
end

local function start()
	local enabled = tonumber(uci:get(CONFIG, "@global[0]", "enable") or 0)
	if enabled == nil or enabled == 0 then
		return
	end
	cmd(string.format("mkdir -p %s %s", CONFIG_PATH, TMP_BIN_PATH))
	cmd(string.format("touch %s", LOG_APP_FILE))
	if nft_flag == "0" then
		ipt("-N PSW-SERVER")
		ipt("-I INPUT -j PSW-SERVER")
		ip6t("-N PSW-SERVER")
		ip6t("-I INPUT -j PSW-SERVER")
	else
		nft_file, err = io.open(NFT_INCLUDE_FILE, "w")
		nft_file:write('#!/usr/sbin/nft -f\n')
		nft_file:write('add chain inet fw4 PSW-SERVER\n')
		nft_file:write('flush chain inet fw4 PSW-SERVER\n')
		nft_file:write('insert rule inet fw4 input position 0 jump PSW-SERVER comment "PSW-SERVER"\n')
	end
	uci:foreach(CONFIG, "user", function(user)
		local id = user[".name"]
		local enable = user.enable
		if enable and tonumber(enable) == 1 then
			local enable_log = user.log
			local log_path = nil
			if enable_log and enable_log == "1" then
				log_path = CONFIG_PATH .. "/" .. id .. ".log"
			else
				log_path = nil
			end
			local remarks = user.remarks
			local port = tonumber(user.port)
			local bin
			local config = {}
			local config_file = CONFIG_PATH .. "/" .. id .. ".json"
			local udp_forward = 1
			local type = user.type or ""
			if type == "Socks" then
				local auth = ""
				if user.auth and user.auth == "1" then
					local username = user.username or ""
					local password = user.password or ""
					if username ~= "" and password ~= "" then
						username = "-u " .. username
						password = "-P " .. password
						auth = username .. " " .. password
					end
				end
				bin = ln_run("/usr/bin/microsocks", "microsocks_" .. id, string.format("-i :: -p %s %s", port, auth), log_path)
			elseif type == "SS" or type == "SSR" then
				config = require(require_dir .. "util_shadowsocks").gen_config_server(user)
				local udp_param = ""
				udp_forward = tonumber(user.udp_forward) or 1
				if udp_forward == 1 then
					udp_param = "-u"
				end
				type = type:lower()
				bin = ln_run("/usr/bin/" .. type .. "-server", type .. "-server", "-c " .. config_file .. " " .. udp_param, log_path)
			elseif type == "SS-Rust" then
				config = require(require_dir .. "util_shadowsocks").gen_config_server(user)
				bin = ln_run("/usr/bin/ssserver", "ssserver", "-c " .. config_file, log_path)
			elseif type == "sing-box" then
				config = require(require_dir .. "util_sing-box").gen_config_server(user)
				bin = ln_run(api.get_app_path("singbox"), "sing-box", "run -c " .. config_file, log_path)
			elseif type == "Xray" then
				config = require(require_dir .. "util_xray").gen_config_server(user)
				bin = ln_run(api.get_app_path("xray"), "xray", "run -c " .. config_file, log_path)
			elseif type == "Trojan" then
				config = require(require_dir .. "util_trojan").gen_config_server(user)
				bin = ln_run("/usr/sbin/trojan", "trojan", "-c " .. config_file, log_path)
			elseif type == "Trojan-Plus" then
				config = require(require_dir .. "util_trojan").gen_config_server(user)
				bin = ln_run("/usr/sbin/trojan-plus", "trojan-plus", "-c " .. config_file, log_path)
			elseif type == "Trojan-Go" then
				config = require(require_dir .. "util_trojan").gen_config_server(user)
				bin = ln_run(api.get_app_path("trojan-go"), "trojan-go", "-config " .. config_file, log_path)
			elseif type == "Brook" then
				local brook_protocol = user.protocol
				local brook_password = user.password
				local brook_path = user.ws_path or "/ws"
				local brook_path_arg = ""
				if brook_protocol == "wsserver" and brook_path then
					brook_path_arg = " --path " .. brook_path
				end
				bin = ln_run(api.get_app_path("brook"), "brook_" .. id, string.format("--debug %s -l :%s -p %s%s", brook_protocol, port, brook_password, brook_path_arg), log_path)
			elseif type == "Hysteria2" then
				config = require(require_dir .. "util_hysteria2").gen_config_server(user)
				bin = ln_run(api.get_app_path("hysteria"), "hysteria", "-c " .. config_file .. " server", log_path)
			end

			if next(config) then
				local f, err = io.open(config_file, "w")
				if f and err == nil then
					f:write(jsonc.stringify(config, 1))
					f:close()
				end
				log(string.format("%s %s 生成配置文件并运行 - %s", remarks, port, config_file))
			end

			if bin then
				cmd(bin)
			end

			local bind_local = user.bind_local or 0
			if bind_local and tonumber(bind_local) ~= 1 then
				if nft_flag == "0" then
					ipt(string.format('-A PSW-SERVER -p tcp --dport %s -m comment --comment "%s" -j ACCEPT', port, remarks))
					ip6t(string.format('-A PSW-SERVER -p tcp --dport %s -m comment --comment "%s" -j ACCEPT', port, remarks))
					if udp_forward == 1 then
						ipt(string.format('-A PSW-SERVER -p udp --dport %s -m comment --comment "%s" -j ACCEPT', port, remarks))
						ip6t(string.format('-A PSW-SERVER -p udp --dport %s -m comment --comment "%s" -j ACCEPT', port, remarks))
					end
				else
					nft_file:write(string.format('add rule inet fw4 PSW-SERVER meta l4proto tcp tcp dport {%s} counter accept comment "%s"\n', port, remarks))
					if udp_forward == 1 then
						nft_file:write(string.format('add rule inet fw4 PSW-SERVER meta l4proto udp udp dport {%s} counter accept comment "%s"\n', port, remarks))
					end
				end
			end
		end
	end)
	if nft_flag == "1" then
		nft_file:write("add rule inet fw4 PSW-SERVER return\n")
		nft_file:close()
		cmd("nft -f " .. NFT_INCLUDE_FILE)
	end
	gen_include()
end

local function stop()
	cmd(string.format("top -bn1 | grep -v 'grep' | grep '%s/' | awk '{print $1}' | xargs kill -9 >/dev/null 2>&1", CONFIG_PATH))
	if nft_flag == "0" then
		ipt("-D INPUT -j PSW-SERVER 2>/dev/null")
		ipt("-F PSW-SERVER 2>/dev/null")
		ipt("-X PSW-SERVER 2>/dev/null")
		ip6t("-D INPUT -j PSW-SERVER 2>/dev/null")
		ip6t("-F PSW-SERVER 2>/dev/null")
		ip6t("-X PSW-SERVER 2>/dev/null")
	else
		local nft_cmd = "handles=$(nft -a list chain inet fw4 input | grep -E \"PSW-SERVER\" | awk -F '# handle ' '{print$2}')\n for handle in $handles; do\n nft delete rule inet fw4 input handle ${handle} 2>/dev/null\n done"
		cmd(nft_cmd)
		cmd("nft flush chain inet fw4 PSW-SERVER 2>/dev/null")
		cmd("nft delete chain inet fw4 PSW-SERVER 2>/dev/null")
	end
	cmd(string.format("rm -rf %s %s /tmp/etc/%s.include", CONFIG_PATH, LOG_APP_FILE, CONFIG))
end

if action then
	if action == "start" then
		start()
	elseif action == "stop" then
		stop()
	end
end
