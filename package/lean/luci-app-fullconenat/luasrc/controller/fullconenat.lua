module("luci.controller.fullconenat", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/fullconenat") then
		return
	end
	local page
	page = entry({"admin", "network", "fullconenat"}, cbi("fullconenat"), _("fullconenat"), 101)
	page.i18n = "fullconenat"
	page.dependent = true
end
