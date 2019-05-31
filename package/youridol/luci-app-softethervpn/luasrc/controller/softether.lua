
module("luci.controller.softether", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/softethervpn") then
		return
	end
	
  entry({"admin", "vpn"}, firstchild(), "VPN", 45).dependent = false
	entry({"admin", "vpn", "softether"}, cbi("softether/server"), _("Softether VPN server"), 89).dependent=false
	entry({"admin", "vpn", "softether","status"},call("act_status")).leaf=true
end

function act_status()
  local e={}
  e.running=luci.sys.call("pgrep vpnserver>/dev/null")==0
  luci.http.prepare_content("application/json")
  luci.http.write_json(e)
end
