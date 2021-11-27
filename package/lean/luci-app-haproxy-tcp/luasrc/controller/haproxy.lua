module("luci.controller.haproxy", package.seeall)

function index()
    if not nixio.fs.access("/etc/config/haproxy") then
		return
	end
	
	entry({"admin", "services", "haproxy"}, cbi("haproxy"), _("HAProxy")).dependent = true
end
