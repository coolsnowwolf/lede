module("luci.controller.devcom",package.seeall)
function index()
	
	local page
	
	page = entry({"admin", "network", "DEVCOM"}, cbi("devcom"), _("设备备注"), 15)
	page.dependent = true
end