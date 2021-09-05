local function scrape()
  for line in io.lines("/proc/meminfo") do
    local name, size, unit = string.match(line, "([^:]+):%s+(%d+)%s?(k?B?)")
    if unit == 'kB' then
      size = size * 1024
    end
    metric("node_memory_"..name:gsub("[):]", ""):gsub("[(]", "_").."_bytes",
        "gauge", nil, size)
  end
end

return { scrape = scrape }
