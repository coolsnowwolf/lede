-- Copyright 2018-2019 Lienol <lawlienol@gmail.com>
module("luci.controller.pppoe-relay", package.seeall)
local uci = require"luci.model.uci".cursor()

function index()
    if not nixio.fs.access("/etc/config/pppoe-relay") then return end
    entry({"admin", "services", "pppoe-relay"}, cbi("pppoe-relay"),
          _("PPPoE Relay"), 2).dependent = true
    entry({"admin", "services", "pppoe-relay", "get_status"}, call("get_status")).leaf =
        true
end

function get_status()
    local json = {}
    json.status = false
    json.index = luci.http.formvalue("index")
    local section = luci.http.formvalue("section")
    local server_interface = uci:get("pppoe-relay", section, "server_interface")
    local client_interface = uci:get("pppoe-relay", section, "client_interface")
    if server_interface and client_interface then
        status = luci.sys.call("ps | grep '/usr/sbin/pppoe-relay -S " ..
                                   server_interface .. " -C " ..
                                   client_interface ..
                                   "' | grep -v 'grep' >/dev/null")
        if status == 0 then json.status = true end
    end
    luci.http.prepare_content("application/json")
    luci.http.write_json(json)
end
