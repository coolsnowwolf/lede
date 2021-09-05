local api = require "luci.model.cbi.passwall.api.api"
local appname = api.appname

local nodes_table = {}
for k, e in ipairs(api.get_valid_nodes()) do
    nodes_table[#nodes_table + 1] = e
end

m = Map(appname)

-- [[ Auto Switch Settings ]]--
s = m:section(TypedSection, "auto_switch")
s.anonymous = true

---- Enable
o = s:option(Flag, "enable", translate("Enable"))
o.default = 0
o.rmempty = false

o = s:option(Value, "testing_time", translate("How often to test"), translate("Units:minutes"))
o.datatype = "uinteger"
o.default = 1

o = s:option(Value, "connect_timeout", translate("Timeout seconds"), translate("Units:seconds"))
o.datatype = "uinteger"
o.default = 3

o = s:option(Value, "retry_num", translate("Timeout retry num"))
o.datatype = "uinteger"
o.default = 3
    
o = s:option(DynamicList, "tcp_node", "TCP " .. translate("List of backup nodes"))
for k, v in pairs(nodes_table) do
    if v.node_type == "normal" then
        o:value(v.id, v["remark"])
    end
end
function o.write(self, section, value)
    local t = {}
    local t2 = {}
    if type(value) == "table" then
		local x
		for _, x in ipairs(value) do
			if x and #x > 0 then
                if not t2[x] then
                    t2[x] = x
                    t[#t+1] = x
                end
			end
		end
	else
		t = { value }
	end
	return DynamicList.write(self, section, t)
end

o = s:option(Flag, "restore_switch", "TCP " .. translate("Restore Switch"), translate("When detects main node is available, switch back to the main node."))

o = s:option(ListValue, "shunt_logic", "TCP " .. translate("If the main node is V2ray/Xray shunt"))
o:value("0", translate("Switch it"))
o:value("1", translate("Applying to the default node"))
o:value("2", translate("Applying to the default preproxy node"))

m:append(Template(appname .. "/auto_switch/footer"))

return m
