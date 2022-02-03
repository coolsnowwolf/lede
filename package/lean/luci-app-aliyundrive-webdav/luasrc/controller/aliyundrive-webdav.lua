module("luci.controller.aliyundrive-webdav", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/aliyundrive-webdav") then
		return
	end

	local page
	page = entry({"admin", "services", "aliyundrive-webdav"}, alias("admin", "services", "aliyundrive-webdav", "client"), _("AliyunDrive WebDAV"), 10) -- 首页
	page.dependent = true
	page.acl_depends = { "luci-app-aliyundrive-webdav" }

	entry({"admin", "services", "aliyundrive-webdav", "client"}, cbi("aliyundrive-webdav/client"), _("Settings"), 10).leaf = true  -- 客户端配置
	entry({"admin", "services", "aliyundrive-webdav", "log"}, form("aliyundrive-webdav/log"), _("Log"), 30).leaf = true -- 日志页面

	entry({"admin", "services", "aliyundrive-webdav", "status"}, call("action_status")).leaf = true -- 运行状态
	entry({"admin", "services", "aliyundrive-webdav", "logtail"}, call("action_logtail")).leaf = true -- 日志采集
end

function action_status()
	local e = {}
	e.running = luci.sys.call("pidof aliyundrive-webdav >/dev/null") == 0
	e.application = luci.sys.exec("aliyundrive-webdav --version")
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end

function action_logtail()
	local fs = require "nixio.fs"
	local log_path = "/var/log/aliyundrive-webdav.log"
	local e = {}
	e.running = luci.sys.call("pidof aliyundrive-webdav >/dev/null") == 0
	if fs.access(log_path) then
		e.log = luci.sys.exec("tail -n 100 %s | sed 's/\\x1b\\[[0-9;]*m//g'" % log_path)
	else
		e.log = ""
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end
