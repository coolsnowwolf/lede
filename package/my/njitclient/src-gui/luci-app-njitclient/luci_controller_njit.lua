module("luci.controller.njit", package.seeall)

function index()
	entry({"admin", "network", "njit"}, cbi("njit"), _("njit-client"), 88).i18n = "wol"
end
