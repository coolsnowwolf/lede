module("luci.controller.brook", package.seeall)
function index()
		if not nixio.fs.access("/etc/config/brook") then
		return
	end
	local page
	page = entry({"admin", "services", "brook"}, cbi("brook"), _("Brook Pro"))
	page.dependent = true
end
