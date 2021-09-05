local ubus = require "ubus"

local function scrape()
  local dsl_line_attenuation = metric("dsl_line_attenuation_db", "gauge")
  local dsl_signal_attenuation = metric("dsl_signal_attenuation_db", "gauge")
  local dsl_snr = metric("dsl_signal_to_noise_margin_db", "gauge")
  local dsl_aggregated_transmit_power = metric("dsl_aggregated_transmit_power_db", "gauge")
  local dsl_latency = metric("dsl_latency_seconds", "gauge")
  local dsl_datarate = metric("dsl_datarate", "gauge")
  local dsl_max_datarate = metric("dsl_max_datarate", "gauge")
  local dsl_error_seconds_total = metric("dsl_error_seconds_total", "counter")
  local dsl_errors_total = metric("dsl_errors_total", "counter")

  local u = ubus.connect()
  local m = u:call("dsl", "metrics", {})

  -- dsl hardware/firmware information
  metric("dsl_info", "gauge", {
    atuc_vendor = m.atu_c.vendor,
    atuc_system_vendor = m.atu_c.system_vendor,
    chipset = m.chipset,
    firmware_version = m.firmware_version,
    api_version = m.api_version,
  }, 1)

  -- dsl line settings information
  metric("dsl_line_info", "gauge", {
    annex = m.annex,
    mode = m.mode,
    profile = m.profile,
  }, 1)

  local dsl_up
  if m.up then
    dsl_up = 1
  else
    dsl_up = 0
  end

  metric("dsl_up", "gauge", {
    detail = m.state,
  }, dsl_up)

  -- dsl line status data
  metric("dsl_uptime_seconds", "gauge", {}, m.uptime)

  -- dsl db measurements
  dsl_line_attenuation({direction="down"}, m.downstream.latn)
  dsl_line_attenuation({direction="up"}, m.upstream.latn)
  dsl_signal_attenuation({direction="down"}, m.downstream.satn)
  dsl_signal_attenuation({direction="up"}, m.upstream.satn)
  dsl_snr({direction="down"}, m.downstream.snr)
  dsl_snr({direction="up"}, m.upstream.snr)
  dsl_aggregated_transmit_power({direction="down"}, m.downstream.actatp)
  dsl_aggregated_transmit_power({direction="up"}, m.upstream.actatp)

  -- dsl performance data
  if m.downstream.interleave_delay ~= nil then
    dsl_latency({direction="down"}, m.downstream.interleave_delay / 1000000)
    dsl_latency({direction="up"}, m.upstream.interleave_delay / 1000000)
  end
  dsl_datarate({direction="down"}, m.downstream.data_rate)
  dsl_datarate({direction="up"}, m.upstream.data_rate)
  dsl_max_datarate({direction="down"}, m.downstream.attndr)
  dsl_max_datarate({direction="up"}, m.upstream.attndr)

  -- dsl errors
  dsl_error_seconds_total({err="forward error correction", loc="near"}, m.errors.near.fecs)
  dsl_error_seconds_total({err="forward error correction", loc="far"}, m.errors.far.fecs)
  dsl_error_seconds_total({err="errored", loc="near"}, m.errors.near.es)
  dsl_error_seconds_total({err="errored", loc="far"}, m.errors.far.es)
  dsl_error_seconds_total({err="severely errored", loc="near"}, m.errors.near.ses)
  dsl_error_seconds_total({err="severely errored", loc="far"}, m.errors.far.ses)
  dsl_error_seconds_total({err="loss of signal", loc="near"}, m.errors.near.loss)
  dsl_error_seconds_total({err="loss of signal", loc="far"}, m.errors.far.loss)
  dsl_error_seconds_total({err="unavailable", loc="near"}, m.errors.near.uas)
  dsl_error_seconds_total({err="unavailable", loc="far"}, m.errors.far.uas)
  dsl_errors_total({err="header error code error", loc="near"}, m.errors.near.hec)
  dsl_errors_total({err="header error code error", loc="far"}, m.errors.far.hec)
  dsl_errors_total({err="non pre-emptive crc error", loc="near"}, m.errors.near.crc_p)
  dsl_errors_total({err="non pre-emptive crc error", loc="far"}, m.errors.far.crc_p)
  dsl_errors_total({err="pre-emptive crc error", loc="near"}, m.errors.near.crcp_p)
  dsl_errors_total({err="pre-emptive crc error", loc="far"}, m.errors.far.crcp_p)
end

return { scrape = scrape }
