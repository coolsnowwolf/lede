module("luci.controller.olsr4", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/olsrd") then
		return
	end

	require("luci.model.uci")
	local uci = luci.model.uci.cursor_state()

	local ol = entry(
		{"admin", "services", "olsrd"},
		cbi("olsr/olsrd"), "OLSR IPv4"
	)
	ol.subindex = true

	entry(
		{"admin", "services", "olsrd", "iface"},
		cbi("olsr/olsrdiface")
	).leaf = true

	entry(
		{"admin", "services", "olsrd", "hna"},
		cbi("olsr/olsrdhna"), _("HNA Announcements")
	)

	oplg = entry(
		{"admin", "services", "olsrd", "plugins"},
		cbi("olsr/olsrdplugins"), _("Plugins")
	)

	oplg.leaf = true
	oplg.subindex = true

	local uci = require("luci.model.uci").cursor()
	uci:foreach("olsrd", "LoadPlugin",
		function (section)
			local lib = section.library
			entry(
				{"admin", "services", "olsrd", "plugins", lib },
				cbi("olsr/olsrdplugins"),
				nil --'Plugin "%s"' % lib:gsub("^olsrd_",""):gsub("%.so.+$","")
			)
		end
	)
end

