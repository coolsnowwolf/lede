#!/usr/bin/lua

local fs = require "nixio.fs"

local function scrape()
  for metrics in fs.glob("/var/prometheus/*.prom") do
    output(get_contents(metrics), '\n')
  end
end

return { scrape = scrape }
