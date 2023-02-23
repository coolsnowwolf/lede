module("luci.controller.pcimodem", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/pcimodem") then
		return
	end

	entry({"admin", "network", "pcimodem"}, cbi("pcimodem"), _("PCI Modem Server"), 80).dependent = false
end
