module("luci.controller.xlnetacc",package.seeall)

function index()
	if not nixio.fs.access("/etc/config/xlnetacc") then
		return
	end
	
	entry({"admin", "services", "xlnetacc"}, firstchild(), _("XLNetAcc")).dependent = false
	entry({"admin", "services", "xlnetacc", "general"}, cbi("xlnetacc"), _("Settings"), 1)
	entry({"admin", "services", "xlnetacc", "log"}, template("xlnetacc/logview"), _("Log"), 2)
	entry({"admin", "services", "xlnetacc", "status"}, call("action_status"))
	entry({"admin", "services", "xlnetacc", "logdata"}, call("action_log"))
end

function action_status()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		run_state=luci.sys.call("ps -w | grep xlnetacc.sh | grep -v grep >/dev/null")==0,
		down_state=nixio.fs.readfile("/var/state/xlnetacc_down_state") or "",
		up_state=nixio.fs.readfile("/var/state/xlnetacc_up_state") or ""
	})
end

function action_log()
	local uci=require "luci.model.uci".cursor()
	local util=require "luci.util"
	local log_data={}

	log_data.syslog=util.trim(util.exec("logread | grep xlnetacc"))
	if uci:get("xlnetacc","general","logging")~="0" then
		log_data.client=nixio.fs.readfile("/var/log/xlnetacc.log") or ""
	end
	uci:unload("xlnetacc")
	luci.http.prepare_content("application/json")
	luci.http.write_json(log_data)
end
