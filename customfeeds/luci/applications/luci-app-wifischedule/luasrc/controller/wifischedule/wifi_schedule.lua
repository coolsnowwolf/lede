-- Copyright (c) 2016, prpl Foundation
--
-- Permission to use, copy, modify, and/or distribute this software for any purpose with or without
-- fee is hereby granted, provided that the above copyright notice and this permission notice appear
-- in all copies.
--
-- THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE
-- INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE
-- FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
-- LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
-- ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
--
-- Author: Nils Koenig <openwrt@newk.it>

module("luci.controller.wifischedule.wifi_schedule", package.seeall)

local fs = require "nixio.fs"
local sys = require "luci.sys"
local template = require "luci.template"
local i18n = require "luci.i18n"

function index()
    if not nixio.fs.access("/etc/config/wifi_schedule") then
        return
    end
    entry({"admin", "services", "wifi_schedule"}, firstchild(), _("Wifi Schedule"), 60).dependent=false
    entry({"admin", "services", "wifi_schedule", "tab_from_cbi"}, cbi("wifischedule/wifi_schedule"), _("Schedule"), 1)
    entry({"admin", "services", "wifi_schedule", "wifi_schedule"}, call("wifi_schedule_log"), _("View Logfile"), 2)
    entry({"admin", "services", "wifi_schedule", "cronjob"}, call("view_crontab"), _("View Cron Jobs"), 3)
end

function wifi_schedule_log()
    local logfile = fs.readfile("/tmp/log/wifi_schedule.log") or ""
    template.render("wifischedule/file_viewer",
        {title = i18n.translate("Wifi Schedule Logfile"), content = logfile})
end

function view_crontab()
    local crontab = fs.readfile("/etc/crontabs/root") or ""
    template.render("wifischedule/file_viewer",
        {title = i18n.translate("Cron Jobs"), content = crontab})
end
