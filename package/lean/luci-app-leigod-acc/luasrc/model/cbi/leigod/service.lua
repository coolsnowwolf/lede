local uci = require "luci.model.uci".cursor()

-- config
m = Map("accelerator")
m.title = translate("Leigod Accelerator Config")
m.description = translate("Control Accelerator Config")

s = m:section(TypedSection, "system")
s.addremove = false
s.anonymous = true

enable = s:option(Flag,"enabled" ,translate("Enable"))
enable.rmempty = false
enable.default = 0

tun = s:option(Flag,"tun" ,translate("Tunnel Mode"))
tun.rmempty = false
tun.default = 0

m:section(SimpleSection).template = "leigod/service"

return m
