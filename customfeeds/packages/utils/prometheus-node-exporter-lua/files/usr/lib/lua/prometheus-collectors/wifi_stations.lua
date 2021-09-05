local ubus = require "ubus"
local iwinfo = require "iwinfo"

local function scrape()
  local metric_wifi_stations = metric("wifi_stations", "gauge")

  local metric_wifi_station_signal = metric("wifi_station_signal_dbm","gauge")

  local metric_wifi_station_inactive = metric('wifi_station_inactive_milliseconds', 'gauge')

  local metric_wifi_station_exp_thr = metric('wifi_station_expected_throughput_kilobits_per_second', 'gauge')

  local metric_wifi_station_tx_bitrate = metric('wifi_station_transmit_kilobits_per_second', 'gauge')
  local metric_wifi_station_rx_bitrate = metric('wifi_station_receive_kilobits_per_second', 'gauge')

  local metric_wifi_station_tx_packets = metric("wifi_station_transmit_packets_total","counter")
  local metric_wifi_station_rx_packets = metric("wifi_station_receive_packets_total","counter")

  local metric_wifi_station_tx_bytes = metric('wifi_station_transmit_bytes_total', 'counter')
  local metric_wifi_station_rx_bytes = metric('wifi_station_receive_bytes_total', 'counter')


  local u = ubus.connect()
  local status = u:call("network.wireless", "status", {})

  for dev, dev_table in pairs(status) do
    for _, intf in ipairs(dev_table['interfaces']) do
      local ifname = intf['ifname']
      if ifname ~= nil then
        local iw = iwinfo[iwinfo.type(ifname)]
        local count = 0

        local assoclist = iw.assoclist(ifname)
        for mac, station in pairs(assoclist) do
          local labels = {
            ifname = ifname,
            mac = mac,
          }
          if station.signal and station.signal ~= 0 then
            metric_wifi_station_signal(labels, station.signal)
          end
          if station.inactive then
            metric_wifi_station_inactive(labels, station.inactive)
          end
          if station.expected_throughput and station.expected_throughput ~= 0 then
            metric_wifi_station_exp_thr(labels, station.expected_throughput)
          end
          if station.tx_rate and station.tx_rate ~= 0 then
            metric_wifi_station_tx_bitrate(labels, station.tx_rate)
          end
          if station.rx_rate and station.rx_rate ~= 0 then
            metric_wifi_station_rx_bitrate(labels, station.rx_rate)
          end
          metric_wifi_station_tx_packets(labels, station.tx_packets)
          metric_wifi_station_rx_packets(labels, station.rx_packets)
          if station.tx_bytes then
            metric_wifi_station_tx_bytes(labels, station.tx_bytes)
          end
          if station.rx_bytes then
            metric_wifi_station_rx_bytes(labels, station.rx_bytes)
          end

          count = count + 1
        end
        metric_wifi_stations({ifname = ifname}, count)
      end
    end
  end
end

return { scrape = scrape }
