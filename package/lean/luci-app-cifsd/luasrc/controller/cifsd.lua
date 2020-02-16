-- Licensed to the public under the Apache License 2.0.

module("luci.controller.cifsd", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/ksmbd") then
		return
	end
	
	entry({"admin","nas"},firstchild(),"NAS",44).dependent=false

	local page

	page = entry({"admin", "nas", "cifsd"}, cbi("cifsd"), _("Network Shares (CIFSD)"))
	page.dependent = true
end

