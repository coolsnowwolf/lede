module("luci.controller.njitclient", package.seeall)

function index()
        entry({"admin", "network", "njitclient"}, cbi("njitclient"), _("NJIT Client"), 100)
        end