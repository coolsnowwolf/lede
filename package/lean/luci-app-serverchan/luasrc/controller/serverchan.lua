module("luci.controller.serverchan",package.seeall)

function index()

	if not nixio.fs.access("/etc/config/serverchan")then
		return
	end

	entry({"admin", "services", "serverchan"}, alias("admin", "services", "serverchan", "setting"),_("微信推送"), 30).dependent = true
	entry({"admin", "services", "serverchan", "setting"}, cbi("serverchan/setting"),_("配置"), 40).leaf = true
	entry({"admin", "services", "serverchan", "advanced"}, cbi("serverchan/advanced"),_("高级设置"), 50).leaf = true
	entry({"admin", "services", "serverchan", "client"}, form("serverchan/client"), "在线设备", 80)
	entry({"admin", "services", "serverchan", "log"}, form("serverchan/log"),_("日志"), 99).leaf = true
	entry({"admin", "services", "serverchan", "get_log"}, call("get_log")).leaf = true
	entry({"admin", "services", "serverchan", "clear_log"}, call("clear_log")).leaf = true
	entry({"admin", "services", "serverchan", "status"}, call("act_status")).leaf=true
end

function act_status()
	local e={}
	e.running=luci.sys.call("busybox ps|grep -v grep|grep -c serverchan >/dev/null")==0
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end

function get_log()
	luci.http.write(luci.sys.exec(
		"[ -f '/tmp/serverchan/serverchan.log' ] && cat /tmp/serverchan/serverchan.log"))
end

function clear_log()
	luci.sys.call("echo '' > /tmp/serverchan/serverchan.log")
end
