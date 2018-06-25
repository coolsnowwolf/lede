module("luci.controller.udp2raw", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/udp2raw") then
		return
	end

	entry({"admin", "services", "udp2raw"},
		firstchild(), _("udp2raw-tunnel")).dependent = false

	entry({"admin", "services", "udp2raw", "general"},
		cbi("udp2raw/general"), _("Settings"), 1)

	entry({"admin", "services", "udp2raw", "servers"},
		arcombine(cbi("udp2raw/servers"), cbi("udp2raw/servers-details")),
		_("Servers Manage"), 2).leaf = true

	entry({"admin", "services", "udp2raw", "status"}, call("action_status"))
end

local function is_running(name)
	return luci.sys.call("pidof %s >/dev/null" %{name}) == 0
end

function action_status()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		running = is_running("udp2raw")
	})
end
