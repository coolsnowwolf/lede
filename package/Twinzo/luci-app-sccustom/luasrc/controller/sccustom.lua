module("luci.controller.sccustom", package.seeall)

function index()
	
	if not nixio.fs.access("/etc/config/sccustom") then
		return
	end

	local page
	
	page = entry({"admin", "services", "sccustom"}, cbi("serverchan/sccustom"), _("SCCUSTOM"), 45)
	page.dependent = true
end
