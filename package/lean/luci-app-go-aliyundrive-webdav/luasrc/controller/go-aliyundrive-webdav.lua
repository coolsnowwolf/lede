-- Copyright (C) 2021 jerryk <jerrykuku@qq.com>
-- This is free software, licensed under the APACHE LICENSE, VERSION 2.0.

module("luci.controller.go-aliyundrive-webdav", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/go-aliyundrive-webdav") then
		return
	end
	entry({"admin", "services", "go-aliyundrive-webdav"}, alias("admin", "services", "go-aliyundrive-webdav", "client"),_("Go AliyunDrive WebDAV"), 10).dependent = true  -- 首页
	entry({"admin", "services", "go-aliyundrive-webdav", "client"}, cbi("go-aliyundrive-webdav/client"),_("Settings"), 1).leaf = true  -- 客户端配置
	entry({"admin", "services", "go-aliyundrive-webdav", "log"}, cbi("go-aliyundrive-webdav/log"),_("Log"), 2).leaf = true  -- 日志页面

	entry({"admin", "services", "go-aliyundrive-webdav", "status"}, call("action_status")).leaf = true

end

function action_status()
	local e = {}
	e.running = luci.sys.call("pidof go-aliyundrive-webdav >/dev/null") == 0
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end
