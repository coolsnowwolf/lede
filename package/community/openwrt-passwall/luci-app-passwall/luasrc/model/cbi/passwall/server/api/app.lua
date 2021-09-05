#!/usr/bin/lua

local action = arg[1]
local api = require "luci.model.cbi.passwall.api.api"
local sys = api.sys
local uci = api.uci
local jsonc = api.jsonc

local CONFIG = "passwall_server"
local CONFIG_PATH = "/var/etc/" .. CONFIG
local LOG_APP_FILE = "/var/log/" .. CONFIG .. ".log"
local TMP_BIN_PATH = CONFIG_PATH .. "/bin"
local require_dir = "luci.model.cbi.passwall.server.api."

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

local function ln_start(s, d, command, output)
    if not output then
        output = "/dev/null"
    end
    d = TMP_BIN_PATH .. "/" .. d
    cmd(string.format('[ ! -f "%s" ] && ln -s %s %s 2>/dev/null', d, s, d))
    return string.format("%s >%s 2>&1 &", d .. " " ..command, output)
end

local function gen_include()
    cmd(string.format("echo '#!/bin/sh' > /var/etc/%s.include", CONFIG))
    local function extract_rules(n, a)
        local _ipt = "iptables"
        if n == "6" then
            _ipt = "ip6tables"
        end
        local result = "*" .. a
        result = result .. "\n" .. sys.exec(_ipt .. '-save -t ' .. a .. ' | grep "PSW-SERVER" | sed -e "s/^-A \\(INPUT\\)/-I \\1 1/"')
        result = result .. "COMMIT"
        return result
    end
    local f, err = io.open("/var/etc/" .. CONFIG .. ".include", "a")
    if f and err == nil then
        f:write('iptables-save -c | grep -v "PSW-SERVER" | iptables-restore -c' .. "\n")
        f:write('iptables-restore -n <<-EOT' .. "\n")
        f:write(extract_rules("4", "filter") .. "\n")
        f:write("EOT" .. "\n")
        f:write('ip6tables-save -c | grep -v "PSW-SERVER" | ip6tables-restore -c' .. "\n")
        f:write('ip6tables-restore -n <<-EOT' .. "\n")
        f:write(extract_rules("6", "filter") .. "\n")
        f:write("EOT" .. "\n")
        f:close()
    end
end

local function start()
    local enabled = tonumber(uci:get(CONFIG, "@global[0]", "enable") or 0)
    if enabled == nil or enabled == 0 then
        return
    end
    cmd(string.format("mkdir -p %s %s", CONFIG_PATH, TMP_BIN_PATH))
    cmd(string.format("touch %s", LOG_APP_FILE))
    cmd("iptables -N PSW-SERVER")
    cmd("iptables -I INPUT -j PSW-SERVER")
    cmd("ip6tables -N PSW-SERVER")
    cmd("ip6tables -I INPUT -j PSW-SERVER")
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
                bin = ln_start("/usr/bin/microsocks", "microsocks_" .. id, string.format("-i :: -p %s %s", port, auth), log_path)
            elseif type == "SS" or type == "SSR" then
                config = require(require_dir .. "shadowsocks").gen_config(user)
                local udp_param = ""
                udp_forward = tonumber(user.udp_forward) or 1
                if udp_forward == 1 then
                    udp_param = "-u"
                end
                type = type:lower()
                bin = ln_start("/usr/bin/" .. type .. "-server", type .. "-server", "-c " .. config_file .. " " .. udp_param, log_path)
            elseif type == "V2ray" then
                config = require(require_dir .. "v2ray").gen_config(user)
                bin = ln_start(api.get_v2ray_path(), "v2ray", "-config=" .. config_file, log_path)
            elseif type == "Xray" then
                config = require(require_dir .. "v2ray").gen_config(user)
                bin = ln_start(api.get_xray_path(), "xray", "-config=" .. config_file, log_path)
            elseif type == "Trojan" then
                config = require(require_dir .. "trojan").gen_config(user)
                bin = ln_start("/usr/sbin/trojan", "trojan", "-c " .. config_file, log_path)
            elseif type == "Trojan-Plus" then
                config = require(require_dir .. "trojan").gen_config(user)
                bin = ln_start("/usr/sbin/trojan-plus", "trojan-plus", "-c " .. config_file, log_path)
            elseif type == "Trojan-Go" then
                config = require(require_dir .. "trojan").gen_config(user)
                bin = ln_start(api.get_trojan_go_path(), "trojan-go", "-config " .. config_file, log_path)
            elseif type == "Brook" then
                local brook_protocol = user.protocol
                local brook_password = user.password
                local brook_path = user.ws_path or "/ws"
                local brook_path_arg = ""
                if brook_protocol == "wsserver" and brook_path then
                    brook_path_arg = " --path " .. brook_path
                end
                bin = ln_start(api.get_brook_path(), "brook_" .. id, string.format("--debug %s -l :%s -p %s%s", brook_protocol, port, brook_password, brook_path_arg), log_path)
            elseif type == "Hysteria" then
                config = require(require_dir .. "hysteria").gen_config(user)
                bin = ln_start(api.get_hysteria_path(), "hysteria", "-c " .. config_file .. " server", log_path)
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
                cmd(string.format('iptables -A PSW-SERVER -p tcp --dport %s -m comment --comment "%s" -j ACCEPT', port, remarks))
                cmd(string.format('ip6tables -A PSW-SERVER -p tcp --dport %s -m comment --comment "%s" -j ACCEPT', port, remarks))
                if udp_forward == 1 then
                    cmd(string.format('iptables -A PSW-SERVER -p udp --dport %s -m comment --comment "%s" -j ACCEPT', port, remarks))
                    cmd(string.format('ip6tables -A PSW-SERVER -p udp --dport %s -m comment --comment "%s" -j ACCEPT', port, remarks))
                end 
            end
        end
    end)
    gen_include()
end

local function stop()
    cmd(string.format("top -bn1 | grep -v 'grep' | grep '%s/' | awk '{print $1}' | xargs kill -9 >/dev/null 2>&1", CONFIG_PATH))
    cmd("iptables -D INPUT -j PSW-SERVER 2>/dev/null")
    cmd("iptables -F PSW-SERVER 2>/dev/null")
    cmd("iptables -X PSW-SERVER 2>/dev/null")
    cmd("ip6tables -D INPUT -j PSW-SERVER 2>/dev/null")
    cmd("ip6tables -F PSW-SERVER 2>/dev/null")
    cmd("ip6tables -X PSW-SERVER 2>/dev/null")
    cmd(string.format("rm -rf %s %s /var/etc/%s.include", CONFIG_PATH, LOG_APP_FILE, CONFIG))
end

if action then
    if action == "start" then
        start()
	elseif action == "stop" then
        stop()
	end
end