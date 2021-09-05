-- Copyright 2018-2019 Lienol <lawlienol@gmail.com>
module("luci.controller.pppoe-server", package.seeall)

function index()
    if not nixio.fs.access("/etc/config/pppoe-server") then return end

    entry({"admin", "services", "pppoe-server"},
          alias("admin", "services", "pppoe-server", "settings"),
          _("PPPoE Server"), 3)
    entry({"admin", "services", "pppoe-server", "settings"},
          cbi("pppoe-server/settings"), _("General Settings"), 10).leaf = true
    entry({"admin", "services", "pppoe-server", "users"},
          cbi("pppoe-server/users"), _("Users Manager"), 20).leaf = true
    entry({"admin", "services", "pppoe-server", "online"},
          cbi("pppoe-server/online"), _("Online Users"), 30).leaf = true
    entry({"admin", "services", "pppoe-server", "status"}, call("status")).leaf =
        true
end

function status()
    local e = {}
    e.status = luci.sys.call("pidof %s >/dev/null" % "pppoe-server") == 0
    luci.http.prepare_content("application/json")
    luci.http.write_json(e)
end
