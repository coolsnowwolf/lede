-- Copyright 2016-2020 Xingwang Liao <kuoruan@gmail.com>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.kcptun", package.seeall)

local http = require "luci.http"
local kcp = require "luci.model.kcptun"

function index()
	if not nixio.fs.access("/etc/config/kcptun") then
		return
	end

	entry({"admin", "services", "kcptun"},
		firstchild(), _("Kcptun Client")).dependent = false

	entry({"admin", "services", "kcptun", "settings"},
		cbi("kcptun/settings"), _("Settings"), 1)

	entry({"admin", "services", "kcptun", "servers"},
		arcombine(cbi("kcptun/servers"), cbi("kcptun/servers-detail")),
		_("Server Manage"), 2).leaf = true

	entry({"admin", "services", "kcptun", "log"},
		template("kcptun/log_view"), _("Log"), 3)

	entry({"admin", "services", "kcptun", "status"}, call("action_status"))

	entry({"admin", "services", "kcptun", "log", "data"}, call("action_log_data"))

	entry({"admin", "services", "kcptun", "log", "clear"}, call("action_log_clear")).leaf = true
end

function action_status()
	local client_file = kcp.get_config_option("client_file")
	local running = kcp.is_running(client_file)

	http.prepare_content("application/json")
	http.write_json({
		client = running
	})
end

function action_log_data()
	local util = require "luci.util"

	local log_data = { client = "", syslog = "" }

	local enable_logging = kcp.get_config_option("enable_logging", "0") == "1"

	if enable_logging then
		local client_log_file = kcp.get_current_log_file("client")
		log_data.client = util.trim(
			util.exec("tail -n 50 %s 2>/dev/null | sed 'x;1!H;$!d;x'" % client_log_file))
	end

	log_data.syslog = util.trim(
		util.exec("logread | grep kcptun | tail -n 50 | sed 'x;1!H;$!d;x'"))

	http.prepare_content("application/json")
	http.write_json(log_data)
end

function action_log_clear(type)
	if type and type ~= "" then
		local fs = require "nixio.fs"

		local log_file = kcp.get_current_log_file(type)

		if fs.access(log_file) then
			fs.writefile(log_file, "")
		else
			http.status(404, "Not found")
			return
		end
	end

	http.prepare_content("application/json")
	http.write_json({ code = 0 })
end
