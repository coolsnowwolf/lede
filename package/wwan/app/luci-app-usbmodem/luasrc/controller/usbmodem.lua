module("luci.controller.usbmodem", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/usbmodem") then
		return
	end

	entry({"admin", "network", "usbmodem"}, cbi("usbmodem"), _("USB Modem Server"), 80).dependent = false
end
