-- Copyright 2020 lwz322 <lwz322@qq.com>
-- Licensed to the public under the MIT License.

module("luci.controller.frps", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/frps") then
		return
	end

	entry({"admin", "services", "frps"}, firstchild(), _("Frps")).dependent = false

	entry({"admin", "services", "frps", "common"}, cbi("frps/common"), _("Settings"), 1)
	entry({"admin", "services", "frps", "server"}, cbi("frps/server"), _("Server"), 2).leaf = true

	entry({"admin", "services", "frps", "status"}, call("action_status"))
end


function action_status()
	local e = {}
	e.running = luci.sys.call("pidof frps >/dev/null") == 0
	e.bin_version = luci.sys.exec("frps -v")
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end
