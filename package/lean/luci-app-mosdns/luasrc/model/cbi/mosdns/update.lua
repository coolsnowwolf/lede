m = Map("mosdns")

s = m:section(TypedSection, "mosdns", translate("Update GeoIP & GeoSite dat"))
s.addremove = false
s.anonymous = true

enable = s:option(Flag, "geo_auto_update", translate("Enable Auto Database Update"))
enable.rmempty = false

o = s:option(ListValue, "geo_update_week_time", translate("Update Cycle"))
o:value("*", translate("Every Day"))
o:value("1", translate("Every Monday"))
o:value("2", translate("Every Tuesday"))
o:value("3", translate("Every Wednesday"))
o:value("4", translate("Every Thursday"))
o:value("5", translate("Every Friday"))
o:value("6", translate("Every Saturday"))
o:value("7", translate("Every Sunday"))
o.default = "*"

update_time = s:option(ListValue, "geo_update_day_time", translate("Update Time"))
for t = 0, 23 do
  update_time:value(t, t..":00")
end
update_time.default = 0

data_update = s:option(Button, "geo_update_database", translate("Database Update"))
data_update.rawhtml = true
data_update.template = "mosdns/geo_update"

return m
