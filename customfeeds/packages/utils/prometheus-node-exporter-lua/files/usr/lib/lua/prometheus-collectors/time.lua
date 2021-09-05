local function scrape()
  -- current time
  metric("node_time_seconds", "gauge", nil, os.time())
end

return { scrape = scrape }
