#!/usr/bin/lua

local json = require "cjson"

local function interpret_suffix(rate)
  local value = string.sub(rate, 1, -2)
  local suffix = string.sub(rate, -1)
  if suffix == "K" then return tonumber(value) * 10^3 end
  if suffix == "M" then return tonumber(value) * 10^6 end
  if suffix == "G" then return tonumber(value) * 10^9 end
  return rate
end

local function scrape()
  local status = json.decode(get_contents("/var/run/bmx6/json/status")).status
  local labels = {
    version = status.version,
    id = status.name,
    address = status.primaryIp
  }

  metric("bmx6_status", "gauge", labels, 1)

  local links = json.decode(get_contents("/var/run/bmx6/json/links")).links
  local metric_bmx6_rxRate = metric("bmx6_link_rxRate","gauge")
  local metric_bmx6_txRate = metric("bmx6_link_txRate","gauge")

  for _, link in pairs(links) do
    local labels = {
      source = status.name,
      target = link.name,
      dev = link.viaDev
    }
    metric_bmx6_rxRate(labels, interpret_suffix(link.rxRate))
    metric_bmx6_txRate(labels, interpret_suffix(link.txRate))
  end
end

return { scrape = scrape }
