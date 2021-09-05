module("luci.controller.attendedsysupgrade", package.seeall)

function index()
        entry({"admin", "system", "attended_sysupgrade"}, template("attendedsysupgrade"), _("Attended Sysupgrade"), 1)
end
