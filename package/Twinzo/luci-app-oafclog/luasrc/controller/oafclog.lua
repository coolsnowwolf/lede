module("luci.controller.oafclog",package.seeall)
function index()
	
	local page
	
	page = entry({"admin", "control", "oafControlLog"}, cbi("oafclog"), _("应用日志"), 46)
	page.dependent = true
end