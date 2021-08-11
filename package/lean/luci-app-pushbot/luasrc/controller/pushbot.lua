module("luci.controller.pushbot",package.seeall)

function index()
	if not nixio.fs.access("/etc/config/pushbot") then
		return
	end

	entry({"admin", "services", "pushbot"}, alias("admin", "services", "pushbot", "setting"),_("全能推送"), 30).dependent = true
	entry({"admin", "services", "pushbot", "setting"}, cbi("pushbot/setting"),_("配置"), 40).leaf = true
	entry({"admin", "services", "pushbot", "advanced"}, cbi("pushbot/advanced"),_("高级设置"), 50).leaf = true
	entry({"admin", "services", "pushbot", "client"}, form("pushbot/client"), "在线设备", 80)
	entry({"admin", "services", "pushbot", "log"}, form("pushbot/log"),_("日志"), 99).leaf = true
	entry({"admin", "services", "pushbot", "get_log"}, call("get_log")).leaf = true
	entry({"admin", "services", "pushbot", "clear_log"}, call("clear_log")).leaf = true
	entry({"admin", "services", "pushbot", "status"}, call("act_status")).leaf = true
end

function act_status()
	local e={}
	e.running=luci.sys.call("busybox ps|grep -v grep|grep -c pushbot >/dev/null")==0
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end

function get_log()
	luci.http.write(luci.sys.exec(
		"[ -f '/tmp/pushbot/pushbot.log' ] && cat /tmp/pushbot/pushbot.log"))
end

function clear_log()
	luci.sys.call("echo '' > /tmp/pushbot/pushbot.log")
end
