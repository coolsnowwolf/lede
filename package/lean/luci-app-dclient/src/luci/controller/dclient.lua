
-- Author: Zeyes <lixize8888@qq.com>

module("luci.controller.dclient", package.seeall)

function index()
        entry({"admin", "network", "dclient"}, firstchild(), _("ZTE 802.1X"), 100).dependent = false
        entry({"admin", "network", "dclient", "config"}, cbi("dclient"), _("802.1X Config"), 1)
        entry({"admin", "network", "dclient", "log_view"}, call("dclient_log"), _("View Logfile"), 2)
end


function dclient_log()
        local logfile = luci.sys.exec("cat /tmp/zte.log")
        luci.template.render("dclient_viewer", { title=luci.i18n.translate("Dlient Log"), content=logfile})
end
