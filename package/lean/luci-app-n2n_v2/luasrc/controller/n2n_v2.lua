--[[
N2N V2 Luci configuration page.Made by 981213
]]--

module("luci.controller.n2n_v2", package.seeall)

function index()
	
	if not nixio.fs.access("/etc/config/n2n_v2") then
		return
	end
	
	entry({"admin", "vpn"}, firstchild(), "VPN", 45).dependent = false
	entry({"admin", "vpn", "n2n_v2", "status"}, call("act_status")).leaf = true
	entry({"admin", "vpn", "n2n_v2"}, cbi("n2n_v2"), _("N2N v2 VPN"), 45).dependent = true
end

function act_status()
	local e = {}
	e.running = luci.sys.call("pgrep edge >/dev/null") == 0
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end
