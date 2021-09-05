local function scrape()
  local loadavg = space_split(get_contents("/proc/loadavg"))

  metric("node_load1", "gauge", nil, loadavg[1])
  metric("node_load5", "gauge", nil, loadavg[2])
  metric("node_load15", "gauge", nil, loadavg[3])
end

return { scrape = scrape }
