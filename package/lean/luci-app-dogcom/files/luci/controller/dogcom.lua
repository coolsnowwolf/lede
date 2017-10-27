-- Copyright (C) 2017 fuyumi <280604399@qq.com>
-- Licensed to the public under the GNU Affero General Public License v3.

module("luci.controller.dogcom", package.seeall)

function index()
    if not nixio.fs.access("/etc/config/dogcom") then
        return
    end
    local page
    page = entry({"admin", "network", "dogcom"}, cbi("dogcom"), _("dogcom"), 100)
    page.i18n = "dogcom"
    page.dependent = true
end