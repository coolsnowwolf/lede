module("luci.controller.pdnsd", package.seeall)
function index()
        if not nixio.fs.access("/etc/config/pdnsd") then
                return
        end
        entry({"admin", "services", "pdnsd"}, cbi("pdnsd"), _("Pdnsd")).dependent = true
end