module("luci.controller.bbr", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/bbr") then
		return
	end
	local page
	page = entry({"admin", "network", "bbr"}, cbi("bbr"), _("bbr"), 101)
	page.i18n = "bbr"
	page.dependent = true
end
