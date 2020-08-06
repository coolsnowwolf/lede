local uci = require"luci.model.uci".cursor()
local appname = "passwall"

local nodes_table = {}
uci:foreach(appname, "nodes", function(e)
    if e.type and e.remarks then
        local remarks = ""
        if e.type == "V2ray" and (e.protocol == "_balancing" or e.protocol == "_shunt") then
            remarks = "%s：[%s] " % {translatef(e.type .. e.protocol), e.remarks}
        else
            remarks = "%s：[%s] %s:%s" % {e.type, e.remarks, e.address, e.port}
        end
        nodes_table[#nodes_table + 1] = {
            id = e[".name"],
            remarks = remarks
         }
    end
end)

m = Map(appname)

-- [[ Auto Switch Settings ]]--
s = m:section(TypedSection, "auto_switch")
s.anonymous = true

---- Enable
o = s:option(Flag, "enable", translate("Enable"))
o.default = 0
o.rmempty = false

---- Testing Time
o = s:option(Value, "testing_time", translate("How often is a diagnosis made"),
             translate("Units:minutes"))
o.default = "3"

---- TCP Node
-- local tcp_node_num = tonumber(m:get("@global_other[0]", "tcp_node_num") or 1)
-- 暂时只支持TCP1
local tcp_node_num = 1
for i = 1, tcp_node_num, 1 do
    o = s:option(DynamicList, "tcp_node" .. i, "TCP " .. i .. " " .. translate("List of backup nodes"), translate("List of backup nodes, the first of which must be the primary node and the others the standby node."))
    for k, v in pairs(nodes_table) do
        o:value(v.id, v.remarks)
    end
end

return m
