module("luci.controller.netdata", package.seeall)

function index()

	entry({"admin", "status", "netdata"}, template("netdata/netdata"), _("NetData"), 10).leaf = true
end