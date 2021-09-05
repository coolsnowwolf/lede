local ubus = require "ubus"
local bit = require "bit"

local function get_wifi_interfaces() -- based on hostapd_stations.lua
  local u = ubus.connect()
  local status = u:call("network.wireless", "status", {})
  local interfaces = {}

  for _, dev_table in pairs(status) do
    for _, intf in ipairs(dev_table['interfaces']) do
      table.insert(interfaces, intf['ifname'])
    end
  end

  return interfaces
end

local function scrape()
  local metric_hostapd_ubus_station_rrm_caps_link_measurement =
    metric("hostapd_ubus_station_rrm_caps_link_measurement", "gauge")
  local metric_hostapd_ubus_station_rrm_caps_neighbor_report =
    metric("hostapd_ubus_station_rrm_caps_neighbor_report", "gauge")
  local metric_hostapd_ubus_station_rrm_caps_beacon_report_passive =
    metric("hostapd_ubus_station_rrm_caps_beacon_report_passive", "gauge")
  local metric_hostapd_ubus_station_rrm_caps_beacon_report_active =
    metric("hostapd_ubus_station_rrm_caps_beacon_report_active", "gauge")
  local metric_hostapd_ubus_station_rrm_caps_beacon_report_table =
    metric("hostapd_ubus_station_rrm_caps_beacon_report_table", "gauge")
  local metric_hostapd_ubus_station_rrm_caps_lci_measurement =
    metric("hostapd_ubus_station_rrm_caps_lci_measurement", "gauge")
  local metric_hostapd_ubus_station_rrm_caps_ftm_range_report =
    metric("hostapd_ubus_station_rrm_caps_ftm_range_report", "gauge")

  local function evaluate_metrics(ifname, freq, station, vals)
    local label_station = {
      ifname = ifname,
      freq = freq,
      station = station
    }
    local rrm_caps_link_measurement = bit.band(bit.lshift(1, 0), vals['rrm'][1]) > 0 and 1 or 0
    local rrm_caps_neighbor_report = bit.band(bit.lshift(1, 1), vals['rrm'][1]) > 0 and 1 or 0
    local rrm_caps_beacon_report_passive = bit.band(bit.lshift(1, 4), vals['rrm'][1]) > 0 and 1 or 0
    local rrm_caps_beacon_report_active = bit.band(bit.lshift(1, 5), vals['rrm'][1]) > 0 and 1 or 0
    local rrm_caps_beacon_report_table = bit.band(bit.lshift(1, 6), vals['rrm'][1]) > 0 and 1 or 0
    local rrm_caps_lci_measurement = bit.band(bit.lshift(1, 4), vals['rrm'][2]) > 0 and 1 or 0
    local rrm_caps_ftm_range_report = bit.band(bit.lshift(1, 2), vals['rrm'][5]) > 0 and 1 or 0

    metric_hostapd_ubus_station_rrm_caps_link_measurement(label_station, rrm_caps_link_measurement)
    metric_hostapd_ubus_station_rrm_caps_neighbor_report(label_station, rrm_caps_neighbor_report)
    metric_hostapd_ubus_station_rrm_caps_beacon_report_passive(label_station, rrm_caps_beacon_report_passive)
    metric_hostapd_ubus_station_rrm_caps_beacon_report_active(label_station, rrm_caps_beacon_report_active)
    metric_hostapd_ubus_station_rrm_caps_beacon_report_table(label_station, rrm_caps_beacon_report_table)

    metric_hostapd_ubus_station_rrm_caps_lci_measurement(label_station, rrm_caps_lci_measurement)
    metric_hostapd_ubus_station_rrm_caps_ftm_range_report(label_station, rrm_caps_ftm_range_report)
  end

  for _, ifname in ipairs(get_wifi_interfaces()) do
    local u = ubus.connect()
    local clients_call = u:call("hostapd." .. ifname, "get_clients", {})

    for client, client_table in pairs(clients_call['clients']) do
      evaluate_metrics(ifname,  clients_call['freq'], client, client_table)
    end
  end
end

return { scrape = scrape }
