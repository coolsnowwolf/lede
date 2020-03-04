local i = require "luci.dispatcher"
local e = require "nixio.fs"
local e = require "luci.sys"
local e = luci.model.uci.cursor()
local o = "v2ray_server"

m = Map(o, translate("V2ray Server"))

t = m:section(TypedSection, "global", translate("Global Settings"))
t.anonymous = true
t.addremove = false
e = t:option(Flag, "enable", translate("Enable"))
e.rmempty = false
t:append(Template("v2ray_server/v2ray"))

t = m:section(TypedSection, "user", translate("Users Manager"))
t.anonymous = true
t.addremove = true
t.template = "cbi/tblsection"
t.extedit = i.build_url("admin", "vpn", o, "config", "%s")
function t.create(t, e)
    local e = TypedSection.create(t, e)
    luci.http.redirect(i.build_url("admin", "vpn", o, "config", e))
end
function t.remove(t, a)
    t.map.proceed = true
    t.map:del(a)
    luci.http.redirect(i.build_url("admin", "vpn", o))
end
e = t:option(Flag, "enable", translate("Enable"))
e.width = "5%"
e.rmempty = false
e = t:option(DummyValue, "status", translate("Status"))
e.template = "v2ray_server/users_status"
e.value = translate("Collecting data...")
e = t:option(DummyValue, "remarks", translate("Remarks"))
e.width = "15%"
e = t:option(DummyValue, "port", translate("Port"))
e.width = "10%"
e = t:option(DummyValue, "protocol", translate("Protocol"))
e.width = "15%"
e.cfgvalue = function(self, section)
    local str = "未知"
    local protocol = m:get(section, "protocol") or ""
    if protocol ~= "" then str = (protocol:gsub("^%l", string.upper)) end
    return str
end
e = t:option(DummyValue, "transport", translate("Transport"))
e.width = "10%"
e.cfgvalue = function(self, section)
    local t = "未知"
    local b = ""
    local protocol = m:get(section, "protocol") or ""
    if protocol == "vmess" then
        b = "transport"
    elseif protocol == "shadowsocks" then
        b = "ss_network"
    end
    local a = m:get(section, b) or ""
    if a == "tcp" then
        t = "TCP"
    elseif a == "udp" then
        t = "UDP"
    elseif a == "tcp,udp" then
        t = "TCP,UDP"
    elseif a == "mkcp" then
        t = "mKCP"
    elseif a == "ws" then
        t = "WebSocket"
    elseif a == "h2" then
        t = "HTTP/2"
    elseif a == "quic" then
        t = "QUIC"
    else
        t = "TCP,UDP"
    end
    return t
end
e = t:option(DummyValue, "password", translate("Password"))
e.width = "30%"
e.cfgvalue = function(self, section)
    local e = ""
    local protocol = m:get(section, "protocol") or ""
    if protocol == "vmess" then
        e = "VMess_id"
    elseif protocol == "shadowsocks" then
        e = "ss_password"
    elseif protocol == "socks" then
        e = "socks_password"
    elseif protocol == "http" then
        e = "http_password"
    end
    local e = m:get(section, e) or ""
    local t = ""
    if type(e) == "table" then
        for a = 1, #e do t = t .. e[a] end
    else
        t = e
    end
    return t
end

m:append(Template("v2ray_server/log"))

m:append(Template("v2ray_server/users_list_status"))
return m

