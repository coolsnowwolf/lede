

m = Map("mwan3helper")
m.title	= translate("MWAN3 Helper")
m.description = translate("MWAN3 Helper generate IPSets")

m:section(SimpleSection).template  = "mwan3helper/mwan3helper_status"

s = m:section(TypedSection, "mwan3helper")
s.addremove = false
s.anonymous = true

o = s:option(Flag, "enabled", translate("Enable GFW IPSet"))
o.rmempty = false
o.description = translate("启用 DNS 防污染并打开GFW IPSet（与SSR的GFW列表模式不能同时开启）")

o = s:option(ListValue, "dns", translate("Anti-pollution DNS Server"))
o:value("208.67.222.222:443", translate("OpenDNS (208.67.222.222)"))
o:value("208.67.220.220:443", translate("OpenDNS (208.67.220.220)"))
o:value("8.8.4.4:53", translate("Google Public DNS (8.8.4.4)"))
o:value("8.8.8.8:53", translate("Google Public DNS (8.8.8.8)"))
o:value("209.244.0.3:53", translate("Level 3 Public DNS (209.244.0.3)"))
o:value("209.244.0.4:53", translate("Level 3 Public DNS (209.244.0.4)"))
o:value("4.2.2.1:53", translate("Level 3 Public DNS (4.2.2.1)"))
o:value("4.2.2.2:53", translate("Level 3 Public DNS (4.2.2.2)"))
o:value("4.2.2.3:53", translate("Level 3 Public DNS (4.2.2.3)"))
o:value("4.2.2.4:53", translate("Level 3 Public DNS (4.2.2.4)"))
o:value("1.1.1.1:53", translate("Cloudflare DNS (1.1.1.1)"))

return m
