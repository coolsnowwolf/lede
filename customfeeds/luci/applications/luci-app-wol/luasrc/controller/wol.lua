module("luci.controller.wol", package.seeall)

function index()
	entry({"admin", "services", "wol"}, form("wol"), _("Wake on LAN"), 90)
	entry({"mini", "services", "wol"}, form("wol"), _("Wake on LAN"), 90)
end
