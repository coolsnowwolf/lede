
local netdevsubstat = {
    "receive_bytes_total",
    "receive_packets_total",
    "receive_errs_total",
    "receive_drop_total",
    "receive_fifo_total",
    "receive_frame_total",
    "receive_compressed_total",
    "receive_multicast_total",
    "transmit_bytes_total",
    "transmit_packets_total",
    "transmit_errs_total",
    "transmit_drop_total",
    "transmit_fifo_total",
    "transmit_colls_total",
    "transmit_carrier_total",
    "transmit_compressed_total"
}

local pattern = "([^%s:]+):%s+(%d+)%s+(%d+)%s+(%d+)%s+(%d+)%s+(%d+)%s+(%d+)%s+(%d+)%s+(%d+)%s+(%d+)%s+(%d+)%s+(%d+)%s+(%d+)%s+(%d+)%s+(%d+)%s+(%d+)%s+(%d+)"

local function scrape()
  local nds_table = {}
  for line in io.lines("/proc/net/dev") do
    local t = {string.match(line, pattern)}
    if #t == 17 then
      nds_table[t[1]] = t
    end
  end
  for i, ndss in ipairs(netdevsubstat) do
    netdev_metric = metric("node_network_" .. ndss, "counter")
    for dev, nds_dev in pairs(nds_table) do
      netdev_metric({device=dev}, nds_dev[i+1])
    end
  end
end

return { scrape = scrape }
