-- Copyright 2020 Weizheng Li <lwz322@qq.com>
-- Licensed to the public under the MIT License.

local http = require "luci.http"
local uci = require "luci.model.uci".cursor()
local sys = require "luci.sys"

module("luci.controller.frps", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/frps") then
		return
	end

	entry({"admin", "services", "frps"},
		firstchild(), _("Frps")).dependent = false

	entry({"admin", "services", "frps", "common"},
		cbi("frps/common"), _("Settings"), 1)

	entry({"admin", "services", "frps", "server"},
		cbi("frps/server"), _("Server"), 2).leaf = true

	entry({"admin", "services", "frps", "status"}, call("action_status"))
end


function action_status()
	local running = false

	local client = uci:get("frps", "main", "client_file")
	if client and client ~= "" then
		local file_name = client:match(".*/([^/]+)$") or ""
		if file_name ~= "" then
			running = sys.call("pidof %s >/dev/null" % file_name) == 0
		end
	end

	http.prepare_content("application/json")
	http.write_json({
		running = running
	})
end
