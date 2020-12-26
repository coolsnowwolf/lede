
module("luci.controller.cifs", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/cifs") then
		return
	end

	local page

	page = entry({"admin", "nas", "cifs"}, cbi("cifs"), _("Mount SMB NetShare"))
	page.dependent = true
end
