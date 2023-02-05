module("luci.controller.gobinetmodem", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/gobinetmodem") then
		return
	end

	entry({"admin", "network", "gobinetmodem"}, cbi("gobinetmodem"), _("Gobinet Modem Server"), 80).dependent = false
end
