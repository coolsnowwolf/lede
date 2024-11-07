module("luci.controller.acc", package.seeall)

function index()
  require("luci.i18n")
  entry({ "admin", "services", "acc" }, alias("admin", "services", "acc", "service"), translate("Leigod Acc"), 50)
  entry({ "admin", "services", "acc", "service" }, cbi("leigod/service"), translate("Leigod Service"), 30).i18n = "acc"
  entry({ "admin", "services", "acc", "device" }, cbi("leigod/device"), translate("Leigod Device"), 50).i18n = "acc"
  entry({ "admin", "services", "acc", "app" }, cbi("leigod/app"), translate("Leigod App"), 60).i18n = "acc"
  entry({ "admin", "services", "acc", "notice" }, cbi("leigod/notice"), translate("Leigod Notice"), 80).i18n = "acc"
  entry({ "admin", "services", "acc", "status" }, call("get_acc_status")).leaf = true
  entry({ "admin", "services", "acc", "start_acc_service" }, call("start_acc_service"))
  entry({ "admin", "services", "acc", "stop_acc_service" }, call("stop_acc_service"))
  entry({ "admin", "services", "acc", "schedule_pause" }, call("schedule_pause"))
end

-- get_acc_status get acc status
function get_acc_status()
  -- util module
  local util      = require "luci.util"
  local uci       = require "luci.model.uci".cursor()
  local translate = luci.i18n.translate
  -- init result
  local resp      = {}
  -- init state
  resp.service    = translate("Acc Service Disabled")
  resp.state      = {}
  -- check if exist
  local exist     = util.exec("ps | grep acc-gw | grep -v grep")
  -- check if program is running
  if exist ~= "" then
    resp.service = translate("Acc Service Enabled")
  end
  -- get uci
  local results = uci:get_all("accelerator")
  for _, typ in pairs({ "Phone", "PC", "Game", "Unknown" }) do
    local state = uci:get("accelerator", typ, "state")
    -- check state
    local state_text = "None"
    if state == nil or state == '0' then
    elseif state == '1' then
      state_text = translate("Acc Catalog Started")
    elseif state == '2' then
      state_text = translate("Acc Catalog Stopped")
    elseif state == '3' then
      state_text = translate("Acc Catalog Paused")
    end
    -- store text
    resp.state[translate(typ .. "_Catalog")] = state_text
  end
  luci.http.prepare_content("application/json")
  luci.http.write_json(resp)
end

-- start_acc_service
function start_acc_service()
  -- util module
  local util      = require "luci.util"
  util.exec("/etc/init.d/acc enable")
  util.exec("/etc/init.d/acc restart")
  local resp = {}
  resp.result = "OK"
  luci.http.prepare_content("application/json")
  luci.http.write_json(resp)  
end

-- stop_acc_service
function stop_acc_service()
  -- util module
  local util      = require "luci.util"
  util.exec("/etc/init.d/acc stop")
  util.exec("/etc/init.d/acc disable")
  local resp = {}
  resp.result = "OK"
  luci.http.prepare_content("application/json")
  luci.http.write_json(resp)  
end

-- schedule_pause
function schedule_pause()
  local util = require "luci.util"
  local uci = require "luci.model.uci".cursor()

  local schedule_enabled = uci:get("accelerator", "system", "schedule_enabled") or "0"
  local pause_time = uci:get("accelerator", "system", "pause_time") or "01:00"
  local username = uci:get("accelerator", "system", "username") or ""
  local password = uci:get("accelerator", "system", "password") or ""

  -- Remove existing cron jobs related to leigod-helper.sh
  util.exec("sed -i '/\\/usr\\/sbin\\/leigod\\/leigod-helper.sh/d' /etc/crontabs/root")

  if schedule_enabled == "1" then
    local hour, minute = pause_time:match("(%d+):(%d+)")
    local cron_time = string.format("%s %s * * * USERNAME='%s' PASSWORD='%s' /usr/sbin/leigod/leigod-helper.sh", tonumber(minute), tonumber(hour), username, password)
    
    -- Add new cron job
    local cron_command = string.format('echo "%s" >> /etc/crontabs/root', cron_time)
    util.exec(cron_command)
    util.exec("/etc/init.d/cron restart")
  end

  local resp = { result = "OK" }
  luci.http.prepare_content("application/json")
  luci.http.write_json(resp)
end
