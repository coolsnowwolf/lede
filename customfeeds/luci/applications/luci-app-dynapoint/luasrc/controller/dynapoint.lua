module("luci.controller.dynapoint", package.seeall)

function index()
   if not nixio.fs.access("/etc/config/dynapoint") then
      return
   end
   entry({"admin", "services", "dynapoint"}, cbi("dynapoint"), _("DynaPoint"))
end

