module("luci.controller.sysuh3c", package.seeall)

function index()
        entry({"admin", "network", "sysuh3c"}, cbi("sysuh3c"), _("SYSU H3C Client"), 100)
        end
