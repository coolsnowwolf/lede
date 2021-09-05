local d = require "luci.dispatcher"
local e = luci.model.uci.cursor()

m = Map("socat", translate("Socat"), translate("Socat is a versatile networking tool named after 'Socket CAT', which can be regarded as an N-fold enhanced version of NetCat"))

s = m:section(NamedSection, "global", "global")
s.anonymous = true
s.addremove = false

o = s:option(Flag, "enable", translate("Enable"))
o.rmempty = false

s = m:section(TypedSection, "config", translate("Port Forwards"))
s.anonymous = true
s.addremove = true
s.template = "cbi/tblsection"
s.extedit = d.build_url("admin", "network", "socat", "config", "%s")
function s.filter(e, t)
    if m:get(t, "protocol") == "port_forwards" then
        return true
    end
end
function s.create(e, t)
    local uuid = string.gsub(luci.sys.exec("echo -n $(cat /proc/sys/kernel/random/uuid)"), "-", "")
    t = uuid
    TypedSection.create(e, t)
    luci.http.redirect(e.extedit:format(t))
end
function s.remove(e, t)
    e.map.proceed = true
    e.map:del(t)
    luci.http.redirect(d.build_url("admin", "network", "socat"))
end

o = s:option(Flag, "enable", translate("Enable"))
o.width = "5%"
o.rmempty = false

o = s:option(DummyValue, "status", translate("Status"))
o.template = "socat/status"
o.value = translate("Collecting data...")

o = s:option(DummyValue, "remarks", translate("Remarks"))

o = s:option(DummyValue, "family", translate("Listen Protocol"))
o.cfgvalue = function(t, n)
    local listen = Value.cfgvalue(t, n) or ""
    local protocol = (m:get(n, "proto") or ""):upper()
    if listen == "" then
        return protocol
    else
        return "IPv" .. listen .. "-" .. protocol
    end
end

o = s:option(DummyValue, "listen_port", translate("Listen port"))

o = s:option(DummyValue, "dest_proto", translate("Destination Protocol"))
o.cfgvalue = function(t, n)
    local listen = Value.cfgvalue(t, n)
    local protocol = listen:sub(0, #listen - 1):upper()
    local ip_type = "IPv" .. listen:sub(#listen)
    return ip_type .. "-" .. protocol
end

o = s:option(DummyValue, "dest_ip", translate("Destination address"))

o = s:option(DummyValue, "dest_port", translate("Destination port"))

o = s:option(Flag, "firewall_accept", translate("Open firewall port"))
o.default = "1"
o.rmempty = false

m:append(Template("socat/list_status"))
return m

