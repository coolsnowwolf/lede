module("luci.controller.hypermodem", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/hypermodem") then
		return
	end

	entry({"admin", "network", "hypermodem"}, cbi("hypermodem"), _("Hyper Modem Server"), 80).dependent = false
end
