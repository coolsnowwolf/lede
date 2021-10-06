
module("luci.controller.openvpn-server", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/openvpn") then
		return
	end
	
	entry({"admin", "vpn"}, firstchild(), "VPN", 45).dependent = false
	entry({"admin", "vpn", "openvpn-server"}, cbi("openvpn-server/openvpn-server"), _("OpenVPN Server"), 80).dependent = false
	entry({"admin", "vpn", "openvpn-server", "status"}, call("act_status")).leaf = true
end

function act_status()
	local e={}
	e.running=luci.sys.call("pgrep openvpn >/dev/null")==0
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end