module("luci.controller.vlmcsd", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/vlmcsd") then
		return
	end
	local page
	page = entry({"admin", "services", "vlmcsd"}, cbi("vlmcsd"), _("KMS Server"), 100)
	page.i18n = "vlmcsd"
	page.dependent = true
end
