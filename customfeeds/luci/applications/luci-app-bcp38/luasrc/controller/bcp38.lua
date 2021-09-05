module("luci.controller.bcp38", package.seeall)

function index()
	entry({"admin", "network", "firewall", "bcp38"},
		cbi("bcp38"),
		_("BCP38"), 50).dependent = false
end
