local ubus = require "ubus"

local function get_devices() -- based on hostapd_stations.lua
    local u = ubus.connect()
    local status = u:call("network.device", "status", {})
    local devices = {}

    for dev, dev_table in pairs(status) do
        table.insert(devices, dev)
    end
    return devices
end

local function get_metric(device)
    local label = {
        device = device
      }
      
    if device == "all" then
        for e in io.lines("/proc/net/snmp6") do
            local snmp6 = space_split(e)
            metric("snmp6_" .. snmp6[1], "counter", label, tonumber(snmp6[2]))
        end
    else
        for e in io.lines("/proc/net/dev_snmp6/" .. device) do
            local snmp6 = space_split(e)
            metric("snmp6_" .. snmp6[1], "counter", label, tonumber(snmp6[2]))
        end
    end
end

local function scrape()
    get_metric("all")
    for _, devicename in ipairs(get_devices()) do
        get_metric(devicename)
    end
end

return { scrape = scrape }
