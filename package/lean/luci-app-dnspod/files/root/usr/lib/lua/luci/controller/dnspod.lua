module("luci.controller.dnspod", package.seeall)

function index()
        entry({"admin", "network", "dnspod"}, cbi("dnspod"), _("动态DNSPOD"), 100)
end
