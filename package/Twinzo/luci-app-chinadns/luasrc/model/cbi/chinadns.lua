--[[
	Chinadns
	Copyright (C) 2015 GuoGuo <gch981213@gmail.com>
]]--

local fs = require "nixio.fs"
local sys = require "luci.sys"
local uci = luci.model.uci.cursor()
local ip_count = sys.exec("cat `uci get chinadns.@chinadns[0].chnroute` | wc -l")

m = Map("chinadns", translate("ChinaDNS"))

m:section(SimpleSection).template  = "chinadns/chinadns_status"

s = m:section(TypedSection, "chinadns", translate("General Setting"),translate("<strong><font color=\"red\">Do not use illegally, compiler is not responsible for any consequences caused by illegal use!!</font></strong>"))
s.addremove = false
s.anonymous = true

enable = s:option(Flag, "enable", translate("Enable"))
enable.default = false
enable.rmempty = false

e = s:option(Value, "boot_time", translate("Boot_time"),translate("Delayed time while booting(seconds)."))
e.default = "10"
e.rmempty = false
t = {10, 20, 30, 50, 80, 120}
for i in ipairs(t)
do
	e:value(t[i])
end

e = s:option(Flag, "autofresh", translate("Autofresh"),translate(" Enable chinadns autorefresh."))
e.default = "1"
e.rmempty = false

e = s:option(Value, "refreshtime", translate("refreshtime"), translate("Chinadns restart interval(min)."))
e.default = "10"
e.rmempty = false
t = {10, 20, 30, 50, 80}
for i in ipairs(t)
do
	e:value(t[i])
end

e = s:option(Flag, "automod", translate("Automod"), translate("Modified config files automatically, do not unselect it unless you know what you are doing."))
e.rmempty = false
e.default = "1"

e = s:option(Flag, "bidirectional ", translate("Enable Bidirectional Filter"), translate("Also filter results inside China from foreign DNS servers"))
e.rmempty = false
e.default = "1"

e = s:option(Value, "port", translate("Local Port"))
e.rmempty = false

e = s:option(Value, "chnroute", translate("CHNRoute File"))
e.rmempty = false
e.default = "/etc/chinadns_chnroute.txt"

e = s:option(Value,"server", translate("Upstream Servers"), translate("Use commas to separate multiple ip address"))
e.optional = false
e.rmempty = false
e:value("114.114.114.114,208.67.222.222:443,8.8.8.8")
e:value("114.114.114.114,8.8.4.4")
e:value("114.114.114.114,8.8.8.8")
e:value("114.114.114.114,208.67.222.222")
e:value("114.114.114.114,208.67.220.220")
e:value("114.114.114.114,178.79.131.110")
e:value("114.114.114.114,199.91.73.222")
e:value("114.114.114.114,127.0.0.1:2053")


config = s:option(TextValue, "config", translate("CHNroute"))
config.rows = 20
config.size = 30
config.wrap = "off"
local locate_chnroute = uci:get_first("chinadns","chinadns","chnroute")
function config.cfgvalue(self, section)
	return nixio.fs.readfile(locate_chnroute)
end

function config.write(self, section, value)
	value = value:gsub("\r\n?", "\n")
	nixio.fs.writefile(locate_chnroute, value)
end

e=s:option(DummyValue,"ip_data",translate("China IP Data")) 
e.rawhtml  = true
e.template = "chinadns/refresh"
e.value =ip_count .. " " .. translate("Records")

return m

