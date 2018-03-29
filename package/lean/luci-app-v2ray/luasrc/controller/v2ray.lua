module("luci.controller.v2ray", package.seeall)
function index()
		if not nixio.fs.access("/etc/config/v2ray") then
		return
	end
	local page
	page = entry({"admin", "services", "v2ray"}, cbi("v2ray"), _("V2Ray Pro"))
	page.dependent = true
end