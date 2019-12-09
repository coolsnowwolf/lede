module("luci.controller.qosv4", package.seeall)

function index()
     require("luci.i18n")
     luci.i18n.loadc("qosv4")
    local fs = luci.fs or nixio.fs
    if not fs.access("/etc/config/qosv4") then
		return
	end
	
	
	local page = entry({"admin", "network", "qosv4"}, cbi("qosv4"), "QOSv4")
	page.i18n = "qosv4"
	page.dependent = true


end
