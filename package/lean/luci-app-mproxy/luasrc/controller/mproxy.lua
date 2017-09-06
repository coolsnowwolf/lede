module("luci.controller.mproxy", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/mproxy") then
		return
	end
	local page
	page = entry({"admin", "services", "mproxy"}, cbi("mproxy"), _("Mproxy Server"), 100)
	page.i18n = "mproxy"
	page.dependent = true
end
