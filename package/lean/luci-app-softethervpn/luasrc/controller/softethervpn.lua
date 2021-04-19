module("luci.controller.softethervpn",package.seeall)

function index()
	if not nixio.fs.access("/etc/config/softethervpn") then
		return
	end
	
	entry({"admin", "vpn"}, firstchild(), "VPN", 45).dependent = false
	entry({"admin", "vpn", "softethervpn"}, cbi("softethervpn"), _("SoftEther VPN"), 50).dependent = true
	entry({"admin", "vpn", "softethervpn", "status"}, call("status")).leaf = true
end

function status()
	local e={}
	e.status=luci.sys.call("pidof %s >/dev/null"%"vpnserver")==0
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end
