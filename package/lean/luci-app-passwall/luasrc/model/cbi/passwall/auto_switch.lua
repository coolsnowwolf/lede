local uci = require"luci.model.uci".cursor()
local api = require "luci.model.cbi.passwall.api.api"
local appname = "passwall"

local n = {}
uci:foreach(appname, "nodes", function(e)
    if e.type and e.remarks and e.address and e.port then
        if (e.type == "V2ray_balancing" or e.type == "V2ray_shunt") or (e.address:match("[\u4e00-\u9fa5]") and e.address:find("%.") and e.address:sub(#e.address) ~= ".") then
            n[e[".name"]] = "%s：[%s] %s:%s" % {e.type, e.remarks, e.address, e.port}
        end
    end
end)

local key_table = {}
for key, _ in pairs(n) do table.insert(key_table, key) end
table.sort(key_table)

m = Map(appname)

-- [[ Auto Switch Settings ]]--
s = m:section(TypedSection, "auto_switch", translate("Auto Switch"))
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
-- local tcp_node_num = tonumber(api.uci_get_type("global_other", "tcp_node_num", 1))
-- 暂时只支持TCP1
local tcp_node_num = 1
for i = 1, tcp_node_num, 1 do
    o = s:option(DynamicList, "tcp_node" .. i,
                 "TCP " .. i .. " " .. translate("List of backup nodes"),
                 translate(
                     "List of backup nodes, the first of which must be the primary node and the others the standby node."))
    for _, key in pairs(key_table) do o:value(key, n[key]) end
end

return m
