
module("luci.controller.pptp-server", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/pptpd") then
		return
	end
	
	entry({"admin", "vpn"}, firstchild(), "VPN", 45).dependent = false
	
	local page

	entry({"admin", "vpn", "pptp-server"}, cbi("pptp-server/pptp-server"), _("PPTP VPN Server"), 80).dependent=false
end
