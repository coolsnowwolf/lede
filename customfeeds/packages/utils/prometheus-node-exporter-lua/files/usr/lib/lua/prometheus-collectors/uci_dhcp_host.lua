local uci=require("uci")

local function scrape()
  local curs=uci.cursor()
  local metric_uci_host = metric("uci_dhcp_host", "gauge")

  curs:foreach("dhcp", "host", function(s)
    if s[".type"] == "host" then
      labels = {name=s["name"], mac=string.upper(s["mac"]), dns=s["dns"], ip=s["ip"]}
      metric_uci_host(labels, 1)
    end
  end)
end

return { scrape = scrape }
