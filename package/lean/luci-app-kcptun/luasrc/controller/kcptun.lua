-- Copyright 2016-2017 Xingwang Liao <kuoruan@gmail.com>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.kcptun", package.seeall)

local http = require "luci.http"
local kcp  = require "luci.model.kcptun"

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

	entry({"admin", "services", "kcptun", "check"}, call("action_check")).leaf = true

	entry({"admin", "services", "kcptun", "update"}, call("action_update")).leaf = true

	entry({"admin", "services", "kcptun", "log", "data"}, call("action_log_data"))

	entry({"admin", "services", "kcptun", "log", "clear"}, call("action_log_clear")).leaf = true
end

local function http_write_json(content)
	http.prepare_content("application/json")
	http.write_json(content or { code = 1 })
end

function action_status()
	local client_file = kcp.get_config_option("client_file")

	http_write_json({
		client = kcp.is_running(client_file)
	})
end

function action_check(type)
	local json = nil
	if type == "kcptun" then
		json = kcp.check_kcptun(http.formvalue("arch"))
	elseif type == "luci" then
		json = kcp.check_luci()
	else
		http.status(500, "Bad address")
		return
	end

	http_write_json(json)
end

function action_update(type)
	local json = nil
	if type == "kcptun" then
		local task = http.formvalue("task")
		if task == "extract" then
			json = kcp.extract_kcptun(http.formvalue("file"), http.formvalue("subfix"))
		elseif task == "move" then
			json = kcp.move_kcptun(http.formvalue("file"))
		else
			json = kcp.download_kcptun(http.formvalue("url"))
		end
	elseif type == "luci" then
		json = kcp.update_luci(http.formvalue("url"), http.formvalue("save"))
	else
		http.status(500, "Bad address")
		return
	end

	http_write_json(json)
end

function action_log_data()
	local util = require "luci.util"

	local log_data = { }

	local enable_logging = kcp.get_config_option("enable_logging", "0") == "1"

	if enable_logging then
		local client_log_file = kcp.get_current_log_file("client")
		log_data.client = util.trim(
			util.exec("tail -n 50 %s 2>/dev/null | sed 'x;1!H;$!d;x'" % client_log_file))
	end

	log_data.syslog = util.trim(
		util.exec("logread | grep kcptun | tail -n 50 | sed 'x;1!H;$!d;x'"))

	http_write_json(log_data)
end

function action_log_clear(type)
	if type and type ~= "" then
		local log_file = kcp.get_current_log_file(type)

		local fs = require "nixio.fs"

		if fs.access(log_file) then
			fs.writefile(log_file, "")
		else
			http.status(404, "Not found")
			return
		end
	end

	http_write_json({ code = 0 })
end
