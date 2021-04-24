module("luci.controller.dnsforwarder", package.seeall)
function index()
	if not nixio.fs.access("/etc/config/dnsforwarder") then
		return
	end
	
	entry({"admin", "services", "dnsforwarder"}, alias("admin", "services", "dnsforwarder", "general"), _("Dnsforwarder")).dependent = true
	entry({"admin", "services", "dnsforwarder", "general"}, cbi("dnsforwarder/general"), _("General") ,10).leaf = true
	entry({"admin", "services", "dnsforwarder", "log"}, cbi("dnsforwarder/log"), _("LOG"), 30).leaf = true
end