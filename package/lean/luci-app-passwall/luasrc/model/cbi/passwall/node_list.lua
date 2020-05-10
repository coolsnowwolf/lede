local d = require "luci.dispatcher"
local fs = require "nixio.fs"
local sys = require "luci.sys"
local uci = require"luci.model.uci".cursor()
local api = require "luci.model.cbi.passwall.api.api"
local appname = "passwall"

m = Map(appname)

-- [[ Other Settings ]]--
s = m:section(TypedSection, "global_other")
s.anonymous = true

---- Auto Ping
o = s:option(Flag, "auto_ping", translate("Auto Ping"),
             translate("This will automatically ping the node for latency"))
o.default = 1

---- Use TCP Detection delay
o = s:option(Flag, "use_tcping", translate("Use TCP Detection delay"),
             translate("This will use tcping replace ping detection of node"))
o.default = 1

---- Concise display nodes
o = s:option(Flag, "compact_display_nodes", translate("Concise display nodes"))
o.default = 0

---- Show Add Mode
o = s:option(Flag, "show_add_mode", translate("Show Add Mode"))
o.default = 1

---- Show group
o = s:option(Flag, "show_group", translate("Show Group"))
o.default = 1

-- [[ Add the node via the link ]]--
s:append(Template("passwall/node_list/link_add_node"))

-- [[ Node List ]]--
s = m:section(TypedSection, "nodes")
-- s.description = translate("Support for more than 10,000 ping nodes and luci does not crash and not slow.")
s.anonymous = true
s.addremove = true
s.template = "cbi/tblsection"
s.extedit = d.build_url("admin", "vpn", "passwall", "node_config", "%s")
function s.create(e, t)
    local e = TypedSection.create(e, t)
    luci.http
        .redirect(d.build_url("admin", "vpn", "passwall", "node_config", e))
end

function s.remove(t, a)
    s.map.proceed = true
    s.map:del(a)
    luci.http.redirect(d.build_url("admin", "vpn", "passwall", "node_list"))
end

if api.uci_get_type("global_other", "show_group", "1") == "1" then
    show_group = s:option(DummyValue, "group", translate("Group"))
    show_group.cfgvalue = function(t, n)
        local group = api.uci_get_type_id(n, "group") or "无"
        return group ~= "" and group or "无"
    end
end

s.sortable = true
-- 简洁模式
if api.uci_get_type("global_other", "compact_display_nodes", "0") == "1" then
    if show_group then show_group.width = "25%" end
    o = s:option(DummyValue, "remarks", translate("Remarks"))
    o.cfgvalue = function(t, n)
        local str = ""
        local is_sub = api.uci_get_type_id(n, "is_sub") or ""
        local group = api.uci_get_type_id(n, "group") or ""
        local remarks = api.uci_get_type_id(n, "remarks") or ""
        local type = api.uci_get_type_id(n, "type") or ""
        local address = api.uci_get_type_id(n, "address") or ""
        local port = api.uci_get_type_id(n, "port") or ""
        str = str .. translate(type) .. "：" .. remarks
        if address ~= "" and port ~= "" then
            local s = " （" .. address .. ":" .. port .. "）"
            str = str .. s
        end
        return str
    end
else
    ---- Add Mode
    if api.uci_get_type("global_other", "show_add_mode", "1") == "1" then
        o = s:option(DummyValue, "add_mode", translate("Add Mode"))
        o.cfgvalue = function(t, n)
            local v = Value.cfgvalue(t, n)
            if v and v ~= '' then
                return v
            else
                return '手动'
            end
            return str
        end
    end

    ---- Type
    o = s:option(DummyValue, "type", translate("Type"))
    o.cfgvalue = function(t, n)
        local v = Value.cfgvalue(t, n)
        if v then return translate(v) end
    end

    ---- Remarks
    o = s:option(DummyValue, "remarks", translate("Remarks"))

    ---- Address
    o = s:option(DummyValue, "address", translate("Address"))
    o.cfgvalue = function(t, n)
        return Value.cfgvalue(t, n) or "---"
    end

    ---- Port
    o = s:option(DummyValue, "port", translate("Port"))
    o.cfgvalue = function(t, n)
        return Value.cfgvalue(t, n) or "---"
    end

    ---- Encrypt Method
    --[[ o = s:option(DummyValue, "encrypt_method", translate("Encrypt Method"))
o.width = "15%"
o.cfgvalue = function(t, n)
    local str = "无"
    local type = api.uci_get_type_id(n, "type") or ""
    if type == "SSR" then
        return api.uci_get_type_id(n, "ssr_encrypt_method")
    elseif type == "SS" then
        return api.uci_get_type_id(n, "ss_encrypt_method")
    elseif type == "V2ray" then
        return api.uci_get_type_id(n, "v2ray_security")
    end
    return str
end--]]
end

---- Ping
o = s:option(DummyValue, "ping", translate("Latency"))
if api.uci_get_type("global_other", "auto_ping", "0") == "0" then
    o.template = "passwall/node_list/ping"
else
    o.template = "passwall/node_list/auto_ping"
end

m:append(Template("passwall/node_list/node_list"))

return m
