#!/usr/bin/lua

local action = arg[1]
local sys = require 'luci.sys'
local jsonc = require "luci.jsonc"
local ucic = require"luci.model.uci".cursor()
local _api = require "luci.model.cbi.passwall.api.api"

local CONFIG = "passwall_server"
local CONFIG_PATH = "/var/etc/" .. CONFIG
local LOG_APP_FILE = "/var/log/" .. CONFIG .. ".log"
local TMP_BIN_PATH = CONFIG_PATH .. "/bin"

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

local function ln_start(s, d, command)
    d = TMP_BIN_PATH .. "/" .. d
    cmd(string.format('[ ! -f "%s" ] && ln -s %s %s 2>/dev/null', d, s, d))
    return string.format("%s >/dev/null 2>&1 &", d .. " " ..command)
end

local function gen_include()
    cmd(string.format("echo '#!/bin/sh' > /var/etc/%s.include", CONFIG))
    local function extract_rules(a)
        local result = "*" .. a
        result = result .. "\n" .. sys.exec('iptables-save -t ' .. a .. ' | grep "PSW-SERVER" | sed -e "s/^-A \\(INPUT\\)/-I \\1 1/"')
        result = result .. "COMMIT"
        return result
    end
    local f, err = io.open("/var/etc/" .. CONFIG .. ".include", "a")
    if f and err == nil then
        f:write('iptables-save -c | grep -v "PSW-SERVER" | iptables-restore -c' .. "\n")
        f:write('iptables-restore -n <<-EOT' .. "\n")
        f:write(extract_rules("filter") .. "\n")
        f:write("EOT" .. "\n")
        f:close()
    end
end

local function start()
    local enabled = tonumber(ucic:get(CONFIG, "@global[0]", "enable") or 0)
    if enabled == nil or enabled == 0 then
        return
    end
    cmd(string.format("mkdir -p %s %s", CONFIG_PATH, TMP_BIN_PATH))
    cmd(string.format("touch %s", LOG_APP_FILE))
    cmd("iptables -N PSW-SERVER")
    cmd("iptables -I INPUT -j PSW-SERVER")
    ucic:foreach(CONFIG, "user", function(user)
        local id = user[".name"]
        local enable = user.enable
        if enable and tonumber(enable) == 1 then
            local remarks = user.remarks
            local port = tonumber(user.port)
            local bin
            local config = {}
            local config_file = CONFIG_PATH .. "/" .. id .. ".json"
            local udp_forward = 1
            local type = user.type or ""
            if type == "SSR" then
                config = require("luci.model.cbi.passwall.server.api.ssr").gen_config(user)
                local udp_param = ""
                udp_forward = tonumber(user.udp_forward) or 1
                if udp_forward == 1 then
                    udp_param = "-u"
                end
                bin = ln_start("/usr/bin/ssr-server", "ssr-server", "-c " .. config_file .. " " .. udp_param)
            elseif type == "V2ray" then
                config = require("luci.model.cbi.passwall.server.api.v2ray").gen_config(user)
                bin = ln_start(_api.get_v2ray_path(), "v2ray", "-config=" .. config_file)
            elseif type == "Trojan" then
                config = require("luci.model.cbi.passwall.server.api.trojan").gen_config(user)
                bin = ln_start("/usr/sbin/trojan-plus", "trojan-plus", "-c " .. config_file)
            elseif type == "Trojan-Go" then
                config = require("luci.model.cbi.passwall.server.api.trojan").gen_config(user)
                bin = ln_start(_api.get_trojan_go_path(), "trojan-go", "-config " .. config_file)
            elseif type == "Brook" then
                local brook_protocol = user.brook_protocol
                local brook_password = user.password
                bin = ln_start(_api.get_brook_path(), "brook_" .. id, string.format("%s -l :%s -p %s", brook_protocol, port, brook_password))
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
                if udp_forward == 1 then
                    cmd(string.format('iptables -A PSW-SERVER -p udp --dport %s -m comment --comment "%s" -j ACCEPT', port, remarks))
                end 
            end
        end
    end)
    gen_include()
end

local function stop()
    cmd(string.format("ps -w | grep -v 'grep' | grep '%s/' | awk '{print $1}' | xargs kill -9 >/dev/null 2>&1 &", CONFIG_PATH))
    cmd("iptables -D INPUT -j PSW-SERVER 2>/dev/null")
    cmd("iptables -F PSW-SERVER 2>/dev/null")
    cmd("iptables -X PSW-SERVER 2>/dev/null")
    cmd(string.format("rm -rf %s %s /var/etc/%s.include", CONFIG_PATH, LOG_APP_FILE, CONFIG))
end

if action then
    if action == "start" then
        start()
	elseif action == "stop" then
        stop()
	end
end