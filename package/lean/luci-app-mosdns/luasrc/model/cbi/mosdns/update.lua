m = Map("mosdns")

s = m:section(TypedSection, "mosdns", translate("Update GeoIP & GeoSite dat"))
s.addremove = false
s.anonymous = true

o = s:option(Flag, "geo_auto_update", translate("Enable Auto Database Update"))
o.rmempty = false

o = s:option(ListValue, "geo_update_week_time", translate("Update Cycle"))
o:value("*", translate("Every Day"))
o:value("1", translate("Every Monday"))
o:value("2", translate("Every Tuesday"))
o:value("3", translate("Every Wednesday"))
o:value("4", translate("Every Thursday"))
o:value("5", translate("Every Friday"))
o:value("6", translate("Every Saturday"))
o:value("7", translate("Every Sunday"))
o.default = "3"

o = s:option(ListValue, "geo_update_day_time", translate("Update Time"))
for t = 0, 23 do
    o:value(t, t..":00")
end
default = 3

o = s:option(Value, "github_proxy", translate("GitHub Proxy"), translate("Update data files with GitHub Proxy, leave blank to disable proxy downloads."))
o:value("https://gh-proxy.com", translate("https://gh-proxy.com"))
o:value("https://ghps.cc", translate("https://ghps.cc"))
o.rmempty = true
o.default = ""

o = s:option(Button, "geo_update_database", translate("Database Update"))
o.rawhtml = true
o.template = "mosdns/mosdns_geo_update"

return m
