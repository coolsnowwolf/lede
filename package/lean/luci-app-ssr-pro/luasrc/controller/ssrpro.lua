module("luci.controller.ssrpro", package.seeall)
function index()
		if not nixio.fs.access("/etc/config/ssrpro") then
		return
	end
	local page
	page = entry({"admin", "services", "ssrpro"}, cbi("ssrpro"), _("ShadowsocksR Pro"))
	page.dependent = true
end
