local api = require "luci.passwall.api"
local appname = api.appname
local uci = api.uci
local fs = require "nixio.fs"
local types_dir = "/usr/lib/lua/luci/model/cbi/passwall/client/type/"

if not arg[1] or not uci:get(appname, arg[1]) then
	luci.http.redirect(api.url("node_list"))
end

m = Map(appname, translate("Node Config"))
m.redirect = api.url()
api.set_apply_on_parse(m)

s = m:section(NamedSection, arg[1], "nodes", "")
s.addremove = false
s.dynamic = false

o = s:option(DummyValue, "passwall", " ")
o.rawhtml  = true
o.template = "passwall/node_list/link_share_man"
o.value = arg[1]

o = s:option(Value, "remarks", translate("Node Remarks"))
o.default = translate("Remarks")
o.rmempty = false

o = s:option(ListValue, "type", translate("Type"))

if api.is_finded("ipt2socks") then
	s.fields["type"]:value("Socks", translate("Socks"))

	o = s:option(Value, "socks_address", translate("Address (Support Domain Name)"))
	o:depends("type", "Socks")
	function o.cfgvalue(self, section)
		return m:get(section, "address")
	end
	function o.write(self, section, value)
		m:set(section, "address", value)
	end

	o = s:option(Value, "socks_port", translate("Port"))
	o.datatype = "port"
	o:depends("type", "Socks")
	function o.cfgvalue(self, section)
		return m:get(section, "port")
	end
	function o.write(self, section, value)
		m:set(section, "port", value)
	end

	o = s:option(Value, "socks_username", translate("Username"))
	o:depends("type", "Socks")
	function o.cfgvalue(self, section)
		return m:get(section, "username")
	end
	function o.write(self, section, value)
		m:set(section, "username", value)
	end
	
	o = s:option(Value, "socks_password", translate("Password"))
	o.password = true
	o:depends("type", "Socks")
	function o.cfgvalue(self, section)
		return m:get(section, "password")
	end
	function o.write(self, section, value)
		m:set(section, "password", value)
	end
end

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
