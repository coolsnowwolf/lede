module("luci.controller.guest-wifi", package.seeall)

function index()
	require("luci.i18n")
	luci.i18n.loadc("guest-wifi")
	if not nixio.fs.access("/etc/config/guest-wifi") then
		return
	end
	
	local page = entry({"admin", "network", "guest-wifi"}, cbi("guest-wifi"), translate("Guest-wifi"), 19)
	page.i18n = "guest-wifi"
	page.dependent = true
	
end
