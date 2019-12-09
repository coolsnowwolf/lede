module("luci.controller.appfilter", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/appfilter") then
		return
	end
	
	local page

	page = entry({"admin", "network", "appfilter"}, cbi("appfilter/appfilter"), _("appfilter"))
	page.dependent = true
end
