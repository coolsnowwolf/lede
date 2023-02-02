module("luci.controller.spdmodem", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/spdmodem") then
		return
	end

	entry({"admin", "network", "spdmodem"}, cbi("spdmodem"), _("SPD Modem Server"), 80).dependent = false
end
