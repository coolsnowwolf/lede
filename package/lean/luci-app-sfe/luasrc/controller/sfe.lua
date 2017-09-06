module("luci.controller.sfe", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/sfe") then
		return
	end
	local page
	page = entry({"admin", "network", "sfe"}, cbi("sfe"), _("SFE Acceleration"), 100)
	page.i18n = "sfe"
	page.dependent = true
end
