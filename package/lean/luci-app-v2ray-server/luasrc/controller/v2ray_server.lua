module("luci.controller.v2ray_server", package.seeall)
local http = require "luci.http"
local v2ray = require "luci.model.cbi.v2ray_server.api.v2ray"

function index()
    if not nixio.fs.access("/etc/config/v2ray_server") then return end
    entry({"admin", "vpn"}, firstchild(), "VPN", 45).dependent = false
    entry({"admin", "vpn", "v2ray_server"}, cbi("v2ray_server/index"),
          _("V2ray Server"), 3).dependent = true
    entry({"admin", "vpn", "v2ray_server", "config"}, cbi("v2ray_server/config")).leaf =
        true

    entry({"admin", "vpn", "v2ray_server", "users_status"},
          call("v2ray_users_status")).leaf = true
    entry({"admin", "vpn", "v2ray_server", "get_log"}, call("get_log")).leaf =
        true
    entry({"admin", "vpn", "v2ray_server", "clear_log"}, call("clear_log")).leaf =
        true
end

local function http_write_json(content)
    http.prepare_content("application/json")
    http.write_json(content or {code = 1})
end

function v2ray_users_status()
    local e = {}
    e.index = luci.http.formvalue("index")
    e.status = luci.sys.call(
                   "ps -w| grep -v grep | grep '/var/etc/v2ray_server/" ..
                       luci.http.formvalue("id") .. "' >/dev/null") == 0
    http_write_json(e)
end

function get_log()
    luci.http.write(luci.sys.exec(
                        "[ -f '/var/log/v2ray_server/app.log' ] && cat /var/log/v2ray_server/app.log"))
end

function clear_log() luci.sys.call("echo '' > /var/log/v2ray_server/app.log") end

