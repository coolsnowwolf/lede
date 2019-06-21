-- Copyright (C) 2019 Anton Chen <contact@antonchen.com>
local sys = require "luci.sys"
local http = require "luci.http"

module("luci.controller.clash", package.seeall)
function index()
    entry({"admin", "services", "clash"},alias("admin", "services", "clash", "general"), _("Clash"), 60).dependent = true
    entry({"admin", "services", "clash", "general"},cbi("clash/general"),_("General Settings"), 10).leaf = true
    entry({"admin", "services", "clash", "subscription"},cbi("clash/subscription"),_("Subscription Settings"), 20).leaf = true
    entry({"admin", "services", "clash", "custom-config"},form("clash/custom-config"),_("Custom Config"), 30).leaf = true
    entry({"admin", "services", "clash", "view-config"},call("view_conf"),_("View Config"), 40).leaf = true
    entry({"admin", "services", "clash", "log"}, call("clash_log"), _("Log"), 50).leaf = true
    entry({"admin", "services", "clash", "status"},call("action_status")).leaf = false
end

function view_conf()
    local nxfs = nixio.fs
    luci.template.render("clash/view-config", {nxfs=nxfs})
end

function clash_log()
    local nxfs = nixio.fs
    luci.template.render("clash/log", {nxfs=nxfs})
end

local function is_running()
    return sys.call("pidof clash > /dev/null") == 0
end

function action_status()
    http.prepare_content("application/json")
    http.write_json({
        clash = is_running()
    })
end
