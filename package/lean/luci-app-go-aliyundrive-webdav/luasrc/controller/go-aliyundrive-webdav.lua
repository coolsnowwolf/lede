-- Copyright (C) 2021 jerryk <jerrykuku@qq.com>
-- This is free software, licensed under the APACHE LICENSE, VERSION 2.0.

module("luci.controller.go-aliyundrive-webdav", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/go-aliyundrive-webdav") then
		return
	end
	entry({"admin", "services", "go-aliyundrive-webdav"}, alias("admin", "services", "go-aliyundrive-webdav", "client"), _("Go AliyunDrive WebDAV"), 10).dependent = true  -- 首页
	entry({"admin", "services", "go-aliyundrive-webdav", "client"}, cbi("go-aliyundrive-webdav/client"), _("Settings"), 1).leaf = true  -- 客户端配置
	entry({"admin", "services", "go-aliyundrive-webdav", "log"}, cbi("go-aliyundrive-webdav/log"), _("Log"), 2).leaf = true -- 日志页面

	entry({"admin", "services", "go-aliyundrive-webdav", "status"}, call("action_status")).leaf = true
	entry({"admin", "services", "go-aliyundrive-webdav", "logtail"}, call("action_logtail")).leaf = true

end

function action_status()
	local e = {}
	e.running = luci.sys.call("pidof go-aliyundrive-webdav >/dev/null") == 0
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end

function action_logtail()
	local fs = require "nixio.fs"
	local log_path = "/var/log/go-aliyundrive-webdav.log"
	local e = {}
	e.running = luci.sys.call("pidof go-aliyundrive-webdav >/dev/null") == 0
	if fs.access(log_path) then
		e.log = luci.sys.exec("tail -n 100 %s | sed 's/\\x1b\\[[0-9;]*m//g'" % log_path)
	else
		e.log = ""
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end
