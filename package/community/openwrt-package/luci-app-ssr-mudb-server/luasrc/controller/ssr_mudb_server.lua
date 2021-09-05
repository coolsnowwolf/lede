-- Copyright 2018-2020 Lienol <lawlienol@gmail.com>
module("luci.controller.ssr_mudb_server", package.seeall)
local http = require "luci.http"
local jsonc = require "luci.jsonc"

function index()
    if not nixio.fs.access("/etc/config/ssr_mudb_server") then return end
    entry({"admin", "vpn"}, firstchild(), "VPN", 45).dependent = false
    if nixio.fs.access("/usr/share/ssr_mudb_server") then
        entry({"admin", "vpn", "ssr_mudb_server"}, cbi("ssr_mudb_server/index"), _("SSR MuDB Server"), 2).dependent = true
    end

    entry({"admin", "vpn", "ssr_mudb_server", "user"}, template("ssr_mudb_server/user")).leaf = true
    entry({"admin", "vpn", "ssr_mudb_server", "config"}, cbi("ssr_mudb_server/config")).leaf = true
    entry({"admin", "vpn", "ssr_mudb_server", "status"}, call("status")).leaf = true
    entry({"admin", "vpn", "ssr_mudb_server", "user_save"}, call("user_save")).leaf = true
    entry({"admin", "vpn", "ssr_mudb_server", "user_list"}, call("user_list")).leaf = true
    entry({"admin", "vpn", "ssr_mudb_server", "user_get"}, call("user_get")).leaf = true
    entry({"admin", "vpn", "ssr_mudb_server", "get_link"}, call("get_link")).leaf = true
    entry({"admin", "vpn", "ssr_mudb_server", "clear_traffic"}, call("clear_traffic")).leaf = true
    entry({"admin", "vpn", "ssr_mudb_server", "clear_traffic_all_users"}, call("clear_traffic_all_users")).leaf = true
    entry({"admin", "vpn", "ssr_mudb_server", "remove_user"}, call("remove_user")).leaf = true
    entry({"admin", "vpn", "ssr_mudb_server", "get_log"}, call("get_log")).leaf = true
    entry({"admin", "vpn", "ssr_mudb_server", "clear_log"}, call("clear_log")).leaf =  true
end

local function http_write_json(content)
    http.prepare_content("application/json")
    http.write_json(content or {code = 1})
end

local function get_config_path()
    return luci.sys.exec("echo -n $(cat /usr/share/ssr_mudb_server/userapiconfig.py | grep 'MUDB_FILE' | cut -d \"'\" -f 2)")
end

local function get_config_json()
    return luci.sys.exec("cat " .. get_config_path()) or "[]"
end

function status()
    local e = {}
    e.status = luci.sys.call("ps -w | grep -v grep | grep '/usr/share/ssr_mudb_server/server.py' >/dev/null") == 0
    http_write_json(e)
end

function get_link()
    local e = {}
    local link = luci.sys.exec("cd /usr/share/ssr_mudb_server && python3 mujson_mgr.py -l -p " .. luci.http.formvalue("port") .. " | sed -n '$p'"):gsub("^%s*(.-)%s*$", "%1")
    if link ~= "" then e.link = link end
    http_write_json(e)
end

function clear_traffic()
    local e = {}
    e.status = luci.sys.call("cd /usr/share/ssr_mudb_server && python3 mujson_mgr.py -c -p '" .. luci.http.formvalue("port") .. "' >/dev/null") == 0
    http_write_json(e)
end

function clear_traffic_all_users()
    local e = {}
    e.status = luci.sys.call("/usr/share/ssr_mudb_server/clear_traffic_all_users.sh >/dev/null") == 0
    http_write_json(e)
end

function user_list()
    luci.http.prepare_content("application/json")
    luci.http.write(get_config_json())
end

function user_save()
    local result = {code = 0}
    local action = luci.http.formvalue("action")
    local json_str = luci.http.formvalue("json")
    if action and action == "add" or action == "edit" then
        local user = jsonc.parse(json_str)
        if user then
            local json = jsonc.parse(get_config_json())
            if json then
                local port = user.port
                local is_exist_port = 0
                for index = 1, table.maxn(json) do
                    if json[index].port == tonumber(port) then
                        is_exist_port = 1
                        if user.old_port and user.old_port == tonumber(port) then
                            is_exist_port = 0
                        end
                        break
                    end
                end
                if is_exist_port == 0 then
                    if action == "add" then
                        local new_user = {
                            enable = tonumber(user.enable) or 0,
                            user = user.user or user.port,
                            port = tonumber(user.port),
                            passwd = user.passwd,
                            method = user.method,
                            protocol = user.protocol,
                            obfs = user.obfs,
                            protocol_param = user.protocol_param or "2",
                            speed_limit_per_con = tonumber(user.speed_limit_per_con) or 0,
                            speed_limit_per_user = tonumber(user.speed_limit_per_user) or 0,
                            forbidden_port = user.forbidden_port or "",
                            transfer_enable = tonumber(user.transfer_enable) or 1073741824,
                            d = 0,
                            u = 0
                        }
                        table.insert(json, new_user)
                    elseif action == "edit" then
                        for index = 1, table.maxn(json) do
                            if json[index].port == tonumber(user.old_port) then
                                json[index].enable = tonumber(user.enable) or 0
                                json[index].user = user.user
                                json[index].port = tonumber(user.port)
                                json[index].passwd = user.passwd
                                json[index].method = user.method
                                json[index].protocol = user.protocol
                                json[index].obfs = user.obfs
                                json[index].protocol_param = user.protocol_param
                                json[index].speed_limit_per_con = tonumber(user.speed_limit_per_con)
                                json[index].speed_limit_per_user = tonumber(user.speed_limit_per_user)
                                json[index].forbidden_port = user.forbidden_port
                                json[index].transfer_enable = tonumber(user.transfer_enable)
                                break
                            end
                        end
                    end
                    local f, err = io.open(get_config_path(), "w")
                    if f and err == nil then
                        f:write(jsonc.stringify(json, 1))
                        f:close()
                        luci.sys.call("/etc/init.d/ssr_mudb_server restart")
                        result = {code = 1}
                    end
                else
                    result.msg = "端口已存在！"
                end
            end
        end
    end
    http_write_json(result)
end

function user_get()
    local result = {}
    local port = luci.http.formvalue("port")
    local str = get_config_json()
    local json = jsonc.parse(str)
    if port and str and json then
        for index = 1, table.maxn(json) do
            local o = json[index]
            if o.port == tonumber(port) then
                result = o
                break
            end
        end
    end
    http_write_json(result)
end

function remove_user()
    local port = luci.http.formvalue("port")
    local str = get_config_json()
    local json = jsonc.parse(str)
    if port and str and json then
        for index = 1, table.maxn(json) do
            local o = json[index]
            if o.port == tonumber(port) then
                json[index] = nil
                break
            end
        end
        local f, err = io.open(get_config_path(), "w")
		if f and err == nil then
			f:write(jsonc.stringify(json, 1))
			f:close()
		end
        luci.http.status = 200
    else
        luci.http.status = 500
    end
end

function get_log()
    luci.http.write(luci.sys.exec("[ -f '/var/log/ssr_mudb_server.log' ] && cat /var/log/ssr_mudb_server.log"))
end

function clear_log()
    luci.sys.call("echo '' > /var/log/ssr_mudb_server.log")
end
