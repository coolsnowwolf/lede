module("luci.controller.firewall", package.seeall)

function index()
	entry({"admin", "network", "firewall"},
		alias("admin", "network", "firewall", "zones"),
		_("Firewall"), 60)

	entry({"admin", "network", "firewall", "zones"},
		arcombine(cbi("firewall/zones"), cbi("firewall/zone-details")),
		_("General Settings"), 10).leaf = true

	entry({"admin", "network", "firewall", "forwards"},
		arcombine(cbi("firewall/forwards"), cbi("firewall/forward-details")),
		_("Port Forwards"), 20).leaf = true

	entry({"admin", "network", "firewall", "rules"},
		arcombine(cbi("firewall/rules"), cbi("firewall/rule-details")),
		_("Traffic Rules"), 30).leaf = true

	entry({"admin", "network", "firewall", "custom"},
		form("firewall/custom"),
		_("Custom Rules"), 40).leaf = true
end
