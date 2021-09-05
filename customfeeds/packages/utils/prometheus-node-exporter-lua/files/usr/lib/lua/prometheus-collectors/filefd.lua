local function scrape()
  local file_nr = space_split(get_contents("/proc/sys/fs/file-nr"))

  metric("node_filefd_allocated", "gauge", nil, file_nr[1])
  metric("node_filefd_maximum", "gauge", nil, file_nr[3])
end

return { scrape = scrape }
