local function scrape()
  -- documetation about nf_conntrack:
  -- https://www.frozentux.net/iptables-tutorial/chunkyhtml/x1309.html
  nat_metric =  metric("node_nat_traffic", "gauge" )
  for e in io.lines("/proc/net/nf_conntrack") do
    -- output(string.format("%s\n",e  ))
    local fields = space_split(e)
    local src, dest, bytes;
    bytes = 0;
    for _, field in ipairs(fields) do
      if src == nil and string.match(field, '^src') then
        src = string.match(field,"src=([^ ]+)");
      elseif dest == nil and string.match(field, '^dst') then
        dest = string.match(field,"dst=([^ ]+)");
      elseif string.match(field, '^bytes') then
        local b = string.match(field, "bytes=([^ ]+)");
        bytes = bytes + b;
        -- output(string.format("\t%d %s",ii,field  ));
      end

    end
    -- local src, dest, bytes = string.match(natstat[i], "src=([^ ]+) dst=([^ ]+) .- bytes=([^ ]+)");
    -- local src, dest, bytes = string.match(natstat[i], "src=([^ ]+) dst=([^ ]+) sport=[^ ]+ dport=[^ ]+ packets=[^ ]+ bytes=([^ ]+)")

    local labels = { src = src, dest = dest }
    -- output(string.format("src=|%s| dest=|%s| bytes=|%s|", src, dest, bytes  ))
    nat_metric(labels, bytes )
  end
end

return { scrape = scrape }
