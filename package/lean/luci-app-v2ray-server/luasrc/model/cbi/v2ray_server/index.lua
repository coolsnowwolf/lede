local ds = require "luci.dispatcher"
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
t.extedit = ds.build_url("admin", "services", o, "config", "%s")
function t.create(t, e)
    local uuid = luci.sys.exec("echo -n $(cat /proc/sys/kernel/random/uuid)") or ""
    uuid = string.gsub(uuid, "-", "")
    local e = TypedSection.create(t, uuid)
    luci.http.redirect(ds.build_url("admin", "services", o, "config", uuid))
end
function t.remove(t, a)
    t.map.proceed = true
    t.map:del(a)
    luci.http.redirect(ds.build_url("admin", "services", o))
end

e = t:option(Flag, "enable", translate("Enable"))
e.width = "5%"
e.rmempty = false

e = t:option(DummyValue, "status", translate("Status"))
e.rawhtml = true
e.cfgvalue = function(t, n)
    return string.format('<font class="users_status" hint="%s">%s</font>', n, translate("Collecting data..."))
end

e = t:option(DummyValue, "remarks", translate("Remarks"))

e = t:option(DummyValue, "port", translate("Port"))
e.width = "10%"

e = t:option(DummyValue, "protocol", translate("Protocol"))
e.cfgvalue = function(self, section)
    local str = "未知"
    local protocol = m:get(section, "protocol") or ""
    if protocol ~= "" then str = (protocol:gsub("^%l", string.upper)) end
    return str
end

m:append(Template("v2ray_server/log"))

m:append(Template("v2ray_server/users_list_status"))
return m

