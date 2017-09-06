
module("luci.controller.ipsec-server", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/ipsec") then
		return
	end
	
	entry({"admin", "vpn"}, firstchild(), "VPN", 45).dependent = false
	
	local page

	entry({"admin", "vpn", "ipsec-server"}, cbi("ipsec-server/ipsec-server"), _("IPSec VPN Server"), 80).dependent=false
end
