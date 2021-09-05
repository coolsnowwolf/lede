module("luci.controller.vpnbypass", package.seeall)
function index()
	if not nixio.fs.access("/etc/config/vpnbypass") then
		return
	end
	entry({"admin", "services", "vpnbypass"}, cbi("vpnbypass"), _("VPN Bypass"))
end
