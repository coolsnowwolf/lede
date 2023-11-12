local api = require "luci.passwall.api"
local fs = require "nixio.fs"
local types_dir = "/usr/lib/lua/luci/model/cbi/passwall/server/type/"

m = Map("passwall_server", translate("Server Config"))
m.redirect = api.url("server")
api.set_apply_on_parse(m)

s = m:section(NamedSection, arg[1], "user", "")
s.addremove = false
s.dynamic = false

o = s:option(Flag, "enable", translate("Enable"))
o.default = "1"
o.rmempty = false

o = s:option(Value, "remarks", translate("Remarks"))
o.default = translate("Remarks")
o.rmempty = false

o = s:option(ListValue, "type", translate("Type"))

local type_table = {}
for filename in fs.dir(types_dir) do
	table.insert(type_table, filename)
end
table.sort(type_table)

for index, value in ipairs(type_table) do
	local p_func = loadfile(types_dir .. value)
	setfenv(p_func, getfenv(1))(m, s)
end

return m
