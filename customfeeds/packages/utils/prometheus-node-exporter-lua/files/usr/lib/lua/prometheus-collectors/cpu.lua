-- stat/cpu collector
local function scrape()
  local stat = get_contents("/proc/stat")

  -- system boot time, seconds since epoch
  metric("node_boot_time_seconds", "gauge", nil,
    string.match(stat, "btime ([0-9]+)"))

  -- context switches since boot (all CPUs)
  metric("node_context_switches_total", "counter", nil,
    string.match(stat, "ctxt ([0-9]+)"))

  -- cpu times, per CPU, per mode
  local cpu_mode = {"user", "nice", "system", "idle", "iowait", "irq",
                    "softirq", "steal", "guest", "guest_nice"}
  local i = 0
  local cpu_metric = metric("node_cpu_seconds_total", "counter")
  while true do
    local cpu = {string.match(stat,
      "cpu"..i.." (%d+) (%d+) (%d+) (%d+) (%d+) (%d+) (%d+) (%d+) (%d+) (%d+)")}
    if #cpu ~= 10 then
      break
    end
    for ii, mode in ipairs(cpu_mode) do
      cpu_metric({cpu="cpu"..i, mode=mode}, cpu[ii] / 100)
    end
    i = i + 1
  end

  -- interrupts served
  metric("node_intr_total", "counter", nil,
    string.match(stat, "intr ([0-9]+)"))

  -- processes forked
  metric("node_forks_total", "counter", nil,
    string.match(stat, "processes ([0-9]+)"))

  -- processes running
  metric("node_procs_running_total", "gauge", nil,
    string.match(stat, "procs_running ([0-9]+)"))

  -- processes blocked for I/O
  metric("node_procs_blocked_total", "gauge", nil,
    string.match(stat, "procs_blocked ([0-9]+)"))
end

return { scrape = scrape }
