
local m
local sys = require "luci.sys"
local uci = luci.model.uci.cursor()



m = Map("clash")
m:section(SimpleSection).template  = "clash/status"
m.pageaction = false

return m

