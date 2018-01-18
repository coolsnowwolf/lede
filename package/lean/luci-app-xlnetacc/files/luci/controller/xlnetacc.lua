module("luci.controller.xlnetacc", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/xlnetacc") then
		return
	end

	entry({"admin", "services", "xlnetacc"},
		firstchild(), _("XLNetAcc")).dependent = false

	entry({"admin", "services", "xlnetacc", "general"},
		cbi("xlnetacc"), _("Settings"), 1)

	entry({"admin", "services", "xlnetacc", "log"},
		template("xlnetacc/logview"), _("Log"), 2)

	entry({"admin", "services", "xlnetacc", "status"}, call("action_status"))
	entry({"admin", "services", "xlnetacc", "logdata"}, call("action_log"))
end

local function is_running(name)
	return luci.sys.call("pidof %s >/dev/null" %{name}) == 0
end

function action_status()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		run_state = is_running("xlnetacc.sh"),
		down_state = nixio.fs.readfile("/var/state/xlnetacc_down_state") or "",
		up_state = nixio.fs.readfile("/var/state/xlnetacc_up_state") or ""
	})
end

function action_log()
	local uci = require "luci.model.uci".cursor()
	local util = require "luci.util"
	local log_data = { }

	log_data.syslog = util.trim(util.exec("logread | grep xlnetacc"))
	if uci:get("xlnetacc", "general", "logging") ~= "0" then
		log_data.client = nixio.fs.readfile("/var/log/xlnetacc.log") or ""
	end
	uci:unload("xlnetacc")

	luci.http.prepare_content("application/json")
	luci.http.write_json(log_data)
end
