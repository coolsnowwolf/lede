local api = require "luci.passwall.api"
local appname = api.appname
local sys = api.sys
local net = require "luci.model.network".init()
local datatypes = api.datatypes

local nodes_table = {}
for k, e in ipairs(api.get_valid_nodes()) do
	if e.node_type == "normal" then
		nodes_table[#nodes_table + 1] = {
			id = e[".name"],
			obj = e,
			remarks = e["remark"]
		}
	end
end

m = Map(appname)
api.set_apply_on_parse(m)

-- [[ Haproxy Settings ]]--
s = m:section(TypedSection, "global_haproxy")
s.anonymous = true

s:append(Template(appname .. "/haproxy/status"))

---- Balancing Enable
o = s:option(Flag, "balancing_enable", translate("Enable Load Balancing"))
o.rmempty = false
o.default = false

---- Console Username
o = s:option(Value, "console_user", translate("Console Username"))
o.default = ""
o:depends("balancing_enable", true)

---- Console Password
o = s:option(Value, "console_password", translate("Console Password"))
o.password = true
o.default = ""
o:depends("balancing_enable", true)

---- Console Port
o = s:option(Value, "console_port", translate("Console Port"), translate(
				 "In the browser input routing IP plus port access, such as:192.168.1.1:1188"))
o.default = "1188"
o:depends("balancing_enable", true)

---- Health Check Type
o = s:option(ListValue, "health_check_type", translate("Health Check Type"))
o.default = "passwall_logic"
o:value("tcp", "TCP")
o:value("passwall_logic", translate("Availability test") .. string.format("(passwall %s)", translate("Inner implement")))
o:depends("balancing_enable", true)

---- Health Check Inter
o = s:option(Value, "health_check_inter", translate("Health Check Inter"), translate("Units:seconds"))
o.default = "60"
o:depends("balancing_enable", true)

o = s:option(DummyValue, "health_check_tips", " ")
o.rawhtml = true
o.cfgvalue = function(t, n)
	return string.format('<span style="color: red">%s</span>', translate("When the availability test is used, the load balancing node will be converted into a Socks node. when node list set customizing, must be a Socks node, otherwise the health check will be invalid."))
end
o:depends("health_check_type", "passwall_logic")

-- [[ Balancing Settings ]]--
s = m:section(TypedSection, "haproxy_config", "",
			  "<font color='red'>" ..
			  translate("Add a node, Export Of Multi WAN Only support Multi Wan. Load specific gravity range 1-256. Multiple primary servers can be load balanced, standby will only be enabled when the primary server is offline! Multiple groups can be set, Haproxy port same one for each group.") ..
			  "\n" .. translate("Note that the node configuration parameters for load balancing must be consistent when use TCP health check type, otherwise it cannot be used normally!") ..
			  "</font>")
s.template = "cbi/tblsection"
s.sortable = true
s.anonymous = true
s.addremove = true

s.create = function(e, t)
	TypedSection.create(e, api.gen_short_uuid())
end

s.remove = function(self, section)
	for k, v in pairs(self.children) do
		v.rmempty = true
		v.validate = nil
	end
	TypedSection.remove(self, section)
end

---- Enable
o = s:option(Flag, "enabled", translate("Enable"))
o.default = 1
o.rmempty = false

---- Node Address
o = s:option(Value, "lbss", translate("Node Address"))
for k, v in pairs(nodes_table) do o:value(v.id, v.remarks) end
o.rmempty = false
o.validate = function(self, value)
	if not value then return nil end
	local t = m:get(value) or nil
	if t and t[".type"] == "nodes" then
		return value
	end
	if datatypes.hostport(value) or datatypes.ip4addrport(value) then
		return value
	end
	if api.is_ipv6addrport(value) then
		return value
	end
	return nil, value
end

---- Haproxy Port
o = s:option(Value, "haproxy_port", translate("Haproxy Port"))
o.datatype = "port"
o.default = 1181
o.rmempty = false

---- Node Weight
o = s:option(Value, "lbweight", translate("Node Weight"))
o.datatype = "uinteger"
o.default = 5
o.rmempty = false

---- Export
o = s:option(ListValue, "export", translate("Export Of Multi WAN"))
o:value(0, translate("Auto"))
local wa = require "luci.tools.webadmin"
wa.cbi_add_networks(o)
o.default = 0
o.rmempty = false

---- Mode
o = s:option(ListValue, "backup", translate("Mode"))
o:value(0, translate("Primary"))
o:value(1, translate("Standby"))
o.rmempty = false

return m
