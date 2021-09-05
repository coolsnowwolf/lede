module("luci.controller.simpleadblock", package.seeall)
function index()
	if not nixio.fs.access("/etc/config/simple-adblock") then
		return
	end
	entry({"admin", "services", "simpleadblock"}, cbi("simpleadblock"), _("Simple AdBlock"))
end
