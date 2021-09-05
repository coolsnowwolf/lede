local ubus = require "ubus"

local function scrape()
  local metric_dawn_ap_channel_utilization_ratio = metric("dawn_ap_channel_utilization_ratio","gauge")
  local metric_dawn_ap_stations_total = metric("dawn_ap_stations_total","gauge")
  local metric_dawn_station_signal_dbm = metric("dawn_station_signal_dbm","gauge")

  local u = ubus.connect()
  local network = u:call("dawn", "get_network", {})

  for ssid, ssid_table in pairs(network) do
    for ap, ap_table in pairs(ssid_table) do

      if (ap_table['local'] == true) then

        local ht_support = (ap_table['ht_support'] == true) and 1 or 0
        local vht_support = (ap_table['vht_support'] == true) and 1 or 0

        local labels = {
          ssid = ssid,
          bssid = ap,
          freq = ap_table['freq'],
          hostname = ap_table['hostname'],
          ht_support = ht_support,
          vht_support = vht_support,
          neighbor_report = ap_table['neighbor_report'],
        }
        metric_dawn_ap_channel_utilization_ratio(labels, ap_table['channel_utilization'] / 255)
        metric_dawn_ap_stations_total(labels, ap_table['num_sta'])

        for client, client_table in pairs(ap_table) do
          if (type(client_table) == "table") then

            local client_ht_support = (client_table['ht'] == true) and 1 or 0
            local client_vht_support = (client_table['vht'] == true) and 1 or 0
            local client_signal = client_table['signal'] or -255

            local labels_client_signal = {
              ssid = ssid,
              bssid = ap,
              mac = client,
              ht_support = client_ht_support,
              vht_support = client_vht_support,
            }
            metric_dawn_station_signal_dbm(labels_client_signal, client_signal)
          end
        end
      end
    end
  end
end

return { scrape = scrape }