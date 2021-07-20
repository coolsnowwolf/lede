
module("luci.controller.pptp-server", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/pptpd") then
		return
	end
	
  entry({"admin", "vpn"}, firstchild(), "VPN", 45).dependent = false
	entry({"admin", "vpn", "pptp-server"}, cbi("pptp-server/pptp-server"), _("PPTP VPN Server"), 80).dependent=false
	entry({"admin", "vpn", "pptp-server","status"},call("act_status")).leaf=true
end

function act_status()
  local e={}
  e.running=luci.sys.call("pgrep pptpd >/dev/null")==0
  luci.http.prepare_content("application/json")
  luci.http.write_json(e)
end
