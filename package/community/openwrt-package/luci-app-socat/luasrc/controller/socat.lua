-- Copyright 2020 Lienol <lawlienol@gmail.com>
module("luci.controller.socat", package.seeall)
local http = require "luci.http"

function index()
    if not nixio.fs.access("/etc/config/socat") then return end

    entry({"admin", "network", "socat"}, cbi("socat/index"), _("Socat"), 100)
    entry({"admin", "network", "socat", "config"}, cbi("socat/config")).leaf = true
    entry({"admin", "network", "socat", "status"}, call("status")).leaf = true
end

function status()
      local e = {}
      e.index = luci.http.formvalue("index")
      e.status = luci.sys.call(string.format("ps -w | grep -v 'grep' | grep '/var/etc/socat/%s' >/dev/null", luci.http.formvalue("id"))) == 0
      http.prepare_content("application/json")
      http.write_json(e)
end
