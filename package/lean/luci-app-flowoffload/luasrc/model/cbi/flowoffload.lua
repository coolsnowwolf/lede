local m,s,o
local SYS  = require "luci.sys"

m = Map("flowoffload")
m.title	= translate("Turbo ACC Acceleration Settings")
m.description = translate("Opensource Linux Flow Offload driver (Fast Path or HWNAT)")
m:append(Template("flow/status"))

s = m:section(TypedSection, "flow")
s.addremove = false
s.anonymous = true

flow = s:option(Flag, "flow_offloading", translate("Enable"))
flow.default = 0
flow.rmempty = false
flow.description = translate("Enable software flow offloading for connections. (decrease cpu load / increase routing throughput)")

hw = s:option(Flag, "flow_offloading_hw", translate("HWNAT"))
hw.default = 0
hw.rmempty = true
hw.description = translate("Enable Hardware NAT (depends on hw capability like MTK 762x)")
hw:depends("flow_offloading", 1)

bbr = s:option(Flag, "bbr", translate("Enable BBR"))
bbr.default = 0
bbr.rmempty = false
bbr.description = translate("Bottleneck Bandwidth and Round-trip propagation time (BBR)")

dns = s:option(Flag, "dns", translate("DNS Acceleration"))
dns.default = 0
dns.rmempty = false
dns.description = translate("Enable DNS Cache Acceleration and anti ISP DNS pollution")

o = s:option(Value, "dns_server", translate("Upsteam DNS Server"))
o.default = "114.114.114.114,114.114.115.115"
o.description = translate("Muitiple DNS server can saperate with ','")
o:depends("dns", 1)

return m
