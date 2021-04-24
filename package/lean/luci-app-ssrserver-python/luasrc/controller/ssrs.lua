module("luci.controller.ssrs",package.seeall)

function index()
	if not nixio.fs.access("/etc/config/ssrs") then
		return
	end

	entry({"admin", "vpn"}, firstchild(), "VPN", 45).dependent = false
	entry({"admin", "vpn", "ssrs"}, cbi("ssrs"), _("SSR Python Server"), 4).dependent = true
	entry({"admin", "vpn", "ssrs", "status"}, call("act_status")).leaf = true
end

function act_status()
	local e={}
	e.running=luci.sys.call("ps -w | grep ssrs.json |grep -v grep >/dev/null") == 0
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end
