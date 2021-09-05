-- Copyright 2020 Lienol <lawlienol@gmail.com>
module("luci.controller.pingos", package.seeall)

function index()
    if not nixio.fs.access("/etc/config/pingos") then return end

    entry({"admin", "nas"}, firstchild(), "NAS", 44).dependent = false
    entry({"admin", "nas", "pingos"}, cbi("pingos"), _("PingOS"), 3).dependent = true
    entry({"admin", "nas", "pingos", "status"}, call("act_status")).leaf = true
end

function act_status()
    local e = {}
    e.status = luci.sys.call("ps -w | grep pingos | grep nginx | grep -v grep > /dev/null") == 0
    luci.http.prepare_content("application/json")
    luci.http.write_json(e)
end
