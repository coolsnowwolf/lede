--[[
LuCI - Lua Configuration Interface
Copyright 2023 jjm2473
]]--

module("luci.controller.admin.luci-fan", package.seeall)

function index()
  require "luci.sys"
  local appname = "luci-fan"
  local defaultpage = nil
  if luci.sys.call("/usr/libexec/fan-control get >/dev/null") == 0 then
    entry({"admin", "system", appname}, cbi("luci-fan"), _("Fan Control"), 60)
  else
    entry({"admin", "system", appname}).dependent = true
  end
  entry({"admin", "system", appname, "get_fan_info"}, call("get_fan_info"), nil).leaf = true
end

function get_fan_info(zone, trip)
  require "luci.util"
  local words
  if zone and trip and string.match(zone, "^thermal_zone%d+$") and string.match(trip, "^%d+$") then
    local fs = require "nixio.fs"
    words = {
      luci.util.trim(fs.readfile("/sys/class/thermal/"..zone.."/temp")),
      luci.util.trim(fs.readfile("/sys/class/thermal/"..zone.."/trip_point_"..trip.."_temp"))
    }
  else
    local sys = require "luci.sys"
    words = luci.util.split(luci.util.trim(sys.exec("/usr/libexec/fan-control temp 2>/dev/null")), " ")
  end
  local zone_temp = tonumber(words[1]) / 1000; -- ˚C
  local fan_on_temp = tonumber(words[2]) / 1000; -- ˚C
  luci.http.status(200, "ok")
  luci.http.prepare_content("application/json")
  luci.http.write_json({zone_temp=zone_temp, fan_on_temp=fan_on_temp})
end
