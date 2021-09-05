local function scrape()
  local count = get_contents("/proc/sys/net/netfilter/nf_conntrack_count")
  local max = get_contents("/proc/sys/net/netfilter/nf_conntrack_max")
  if count ~= "" then
    metric("node_nf_conntrack_entries", "gauge", nil, string.sub(count, 1, -2))
  end
  if max ~= "" then
    metric("node_nf_conntrack_entries_limit", "gauge", nil, string.sub(max, 1, -2))
  end
end

return { scrape = scrape }
