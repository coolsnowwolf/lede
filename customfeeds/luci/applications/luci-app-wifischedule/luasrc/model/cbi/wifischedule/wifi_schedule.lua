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

local fs  = require "nixio.fs"
local sys = require "luci.sys"
local uci = require("luci.model.uci").cursor()

function time_validator(self, value, desc)
    if value ~= nil then
        h_str, m_str = string.match(value, "^(%d%d?):(%d%d?)$")
        h = tonumber(h_str)
        m = tonumber(m_str)
        if ( h ~= nil and
             h >= 0   and
             h <= 23  and
             m ~= nil and
             m >= 0   and
             m <= 59) then
            return value
        end
    end
    return nil, translatef("The value %s is invalid", desc)
end

-- -------------------------------------------------------------------------------------------------

-- BEGIN Map
m = Map("wifi_schedule", translate("Wifi Schedule"), translate("Defines a schedule when to turn on and off wifi."))
function m.on_commit(self)
    sys.exec("/usr/bin/wifi_schedule.sh cron")
end
-- END Map

-- BEGIN Global Section
global_section = m:section(TypedSection, "global", translate("Global Settings"))
global_section.optional = false
global_section.rmempty = false
global_section.anonymous = true
-- END Section

-- BEGIN Global Enable Checkbox
global_enable = global_section:option(Flag, "enabled", translate("Enable Wifi Schedule"))
global_enable.optional = false
global_enable.rmempty = false

function global_enable.validate(self, value, global_section)
    if value == "1" then
        if ( fs.access("/sbin/wifi") and
             fs.access("/usr/bin/wifi_schedule.sh") )then
            return value
        else
            return nil, translate("Could not find required /usr/bin/wifi_schedule.sh or /sbin/wifi")
        end
    else
        return "0"
    end
end
-- END Global Enable Checkbox

-- BEGIN Global Logging Checkbox
global_logging = global_section:option(Flag, "logging", translate("Enable logging"))
global_logging.optional = false
global_logging.rmempty = false
global_logging.default = 0
-- END Global Enable Checkbox

-- BEGIN Global Activate WiFi Button
enable_wifi = global_section:option(Button, "enable_wifi", translate("Activate wifi"))
function enable_wifi.write()
    sys.exec("/usr/bin/wifi_schedule.sh start manual")
end
-- END Global Activate Wifi Button

-- BEGIN Global Disable WiFi Gracefully Button
disable_wifi_gracefully = global_section:option(Button, "disable_wifi_gracefully", translate("Disable wifi gracefully"))
function disable_wifi_gracefully.write()
    sys.exec("/usr/bin/wifi_schedule.sh stop manual")
end
-- END Global Disable Wifi Gracefully Button

-- BEGIN Disable WiFi Forced Button
disable_wifi_forced = global_section:option(Button, "disable_wifi_forced", translate("Disabled wifi forced"))
function disable_wifi_forced.write()
    sys.exec("/usr/bin/wifi_schedule.sh forcestop manual")
end
-- END Global Disable WiFi Forced Button

-- BEGIN Global Unload Modules Checkbox
global_unload_modules = global_section:option(Flag, "unload_modules", translate("Unload Modules (experimental; saves more power)"))
global_unload_modules.optional = false
global_unload_modules.rmempty = false
global_unload_modules.default = 0
-- END Global Unload Modules Checkbox


-- BEGIN Modules
modules = global_section:option(TextValue, "modules", "")
modules:depends("unload_modules", global_unload_modules.enabled);
modules.wrap = "off"
modules.rows = 10

function modules.cfgvalue(self, section)
    mod = uci:get("wifi_schedule", section, "modules")
    if mod == nil then
        mod = ""
    end
    return mod:gsub(" ", "\r\n")
end

function modules.write(self, section, value)
    if value then
        value_list = value:gsub("\r\n", " ")
        ListValue.write(self, section, value_list)
        uci:set("wifi_schedule", section, "modules", value_list)
    end
end
-- END Modules

-- BEGIN Determine Modules
determine_modules = global_section:option(Button, "determine_modules", translate("Determine Modules Automatically"))
determine_modules:depends("unload_modules", global_unload_modules.enabled);
function determine_modules.write(self, section)
    output = sys.exec("/usr/bin/wifi_schedule.sh getmodules")
    modules:write(section, output)
end
-- END Determine Modules

-- BEGIN Section
d = m:section(TypedSection, "entry", translate("Schedule events"))
d.addremove = true
--d.anonymous = true
-- END Section

-- BEGIN Enable Checkbox
c = d:option(Flag, "enabled", translate("Enable"))
c.optional = false
c.rmempty = false
-- END Enable Checkbox

-- BEGIN Day(s) of Week
dow = d:option(MultiValue, "daysofweek", translate("Day(s) of Week"))
dow.optional = false
dow.rmempty = false
dow:value("Monday", translate("Monday"))
dow:value("Tuesday", translate("Tuesday"))
dow:value("Wednesday", translate("Wednesday"))
dow:value("Thursday", translate("Thursday"))
dow:value("Friday", translate("Friday"))
dow:value("Saturday", translate("Saturday"))
dow:value("Sunday", translate("Sunday"))
-- END Day(s) of Weel

-- BEGIN Start Wifi Dropdown
starttime =  d:option(Value, "starttime", translate("Start WiFi"))
starttime.optional = false
starttime.rmempty = false
starttime:value("00:00")
starttime:value("01:00")
starttime:value("02:00")
starttime:value("03:00")
starttime:value("04:00")
starttime:value("05:00")
starttime:value("06:00")
starttime:value("07:00")
starttime:value("08:00")
starttime:value("09:00")
starttime:value("10:00")
starttime:value("11:00")
starttime:value("12:00")
starttime:value("13:00")
starttime:value("14:00")
starttime:value("15:00")
starttime:value("16:00")
starttime:value("17:00")
starttime:value("18:00")
starttime:value("19:00")
starttime:value("20:00")
starttime:value("21:00")
starttime:value("22:00")
starttime:value("23:00")

function starttime.validate(self, value, d)
    return time_validator(self, value, translate("Start Time"))
end
-- END Start Wifi Dropdown

-- BEGIN Stop Wifi Dropdown
stoptime = d:option(Value, "stoptime", translate("Stop WiFi"))
stoptime.optional = false
stoptime.rmempty = false
stoptime:value("00:00")
stoptime:value("01:00")
stoptime:value("02:00")
stoptime:value("03:00")
stoptime:value("04:00")
stoptime:value("05:00")
stoptime:value("06:00")
stoptime:value("07:00")
stoptime:value("08:00")
stoptime:value("09:00")
stoptime:value("10:00")
stoptime:value("11:00")
stoptime:value("12:00")
stoptime:value("13:00")
stoptime:value("14:00")
stoptime:value("15:00")
stoptime:value("16:00")
stoptime:value("17:00")
stoptime:value("18:00")
stoptime:value("19:00")
stoptime:value("20:00")
stoptime:value("21:00")
stoptime:value("22:00")
stoptime:value("23:00")

function stoptime.validate(self, value, d)
    return time_validator(self, value, translate("Stop Time"))
end
-- END Stop Wifi Dropdown

-- BEGIN Force Wifi Stop Checkbox
force_wifi = d:option(Flag, "forcewifidown", translate("Force disabling wifi even if stations associated"))
force_wifi.default = false
force_wifi.rmempty = false

function force_wifi.validate(self, value, d)
    if value == "0" then
        if fs.access("/usr/bin/iwinfo") then
            return value
        else
            return nil, translate("Could not find required programm /usr/bin/iwinfo")
        end
    else
        return "1"
    end
end
-- END Force Wifi Checkbox

return m
