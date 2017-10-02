module("luci.controller.haproxy-tcp", package.seeall)

function index()
  entry(
    {"admin", "services", "haproxy-tcp"},
    cbi("haproxy-tcp"), _("HAProxy-TCP"), 55)
end
