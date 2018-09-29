module("luci.controller.flowoffload", package.seeall)

function index()
	if not nixio.fs.access("/sys/module/xt_FLOWOFFLOAD/refcnt") then
		return
	end
	local page
	page = entry({"admin", "network", "flowoffload"}, cbi("flowoffload"), _(" Flow Offload ACC"), 101)
	page.i18n = "flowoffload"
	page.dependent = true
	
	entry({"admin", "network", "flowoffload", "status"}, call("action_status"))
end

local function is_running()
	return luci.sys.call("iptables --list | grep FLOWOFFLOAD >/dev/null") == 0
end

local function is_bbr()
	return luci.sys.call("sysctl net.ipv4.tcp_congestion_control | grep bbr >/dev/null") == 0
end

local function is_fullcone()
	return luci.sys.call("iptables -t nat -L -n --line-numbers | grep FULLCONENAT >/dev/null") == 0
end

function action_status()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		run_state = is_running(),
		down_state = is_bbr(),
		up_state = is_fullcone()
	})
end


