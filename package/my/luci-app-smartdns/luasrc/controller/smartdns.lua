-- Copyright 2018 Nick Peng (pymumu@gmail.com)

module("luci.controller.smartdns", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/smartdns") then
		return
	end

	local page

	page = entry({"admin", "services", "smartdns"}, cbi("smartdns"), _("SmartDNS"), 60)
	page.dependent = true
end
