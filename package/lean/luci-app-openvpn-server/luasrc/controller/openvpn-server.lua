
module("luci.controller.openvpn-server", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/openvpn") then
		return
	end
	
	entry({"admin", "vpn"}, firstchild(), "VPN", 45).dependent = false
	
	local page

	entry({"admin", "vpn", "openvpn-server"}, cbi("openvpn-server/openvpn-server"), _("OpenVPN Server"), 80).dependent=false
end
