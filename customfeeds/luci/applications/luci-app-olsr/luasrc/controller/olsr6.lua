module("luci.controller.olsr6", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/olsrd6") then
		return
	end

	require("luci.model.uci")
	local uci = luci.model.uci.cursor_state()

	local ol = entry(
		{"admin", "services", "olsrd6"},
		cbi("olsr/olsrd6"), "OLSR IPv6"
	)
	ol.subindex = true

	entry(
		{"admin", "services", "olsrd6", "iface"},
		cbi("olsr/olsrdiface6")
	).leaf = true

	entry(
		{"admin", "services", "olsrd6", "hna"},
		cbi("olsr/olsrdhna6"), _("HNA6 Announcements")
	)

	oplg = entry(
		{"admin", "services", "olsrd6", "plugins"},
		cbi("olsr/olsrdplugins6"), _("Plugins")
	)

	oplg.leaf = true
	oplg.subindex = true

	local uci = require("luci.model.uci").cursor()
	uci:foreach("olsrd6", "LoadPlugin",
		function (section)
			local lib = section.library
			entry(
				{"admin", "services", "olsrd6", "plugins", lib },
				cbi("olsr/olsrdplugins6"),
				nil --'Plugin "%s"' % lib:gsub("^olsrd_",""):gsub("%.so.+$","")
			)
		end
	)
end

