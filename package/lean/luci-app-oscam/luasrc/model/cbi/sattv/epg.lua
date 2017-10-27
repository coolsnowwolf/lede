require("luci.tools.webadmin")

--[[
config epg_set
	option enable      1
	option time   30
	option src_url  http://dm.epg.net.cn/a/epg.xml
	option src_url1  http://dm1.epg.net.cn/a/epg.xml
  option src_url2  http://epg.xltvrobbs.net/atmosphere/epg/epg.xml
  option src_url3   http://epg1.xltvrobbs.net/atmosphere/epg/epg.xml

]]--


m = Map("epg", translate("epg_title","Set epg download address"),translate("epg_desc",
"Set epg auto download for dm500 atmosphere"))

s = m:section(TypedSection, "epg_set", translate("epgset","settings"))
s.anonymous = true
s.addremove = false

enable = s:option(Flag, "enable", translate("enable", "enable"))
enable.default = false
enable.optional = false
enable.rmempty = false

interval = s:option(Value, "interval", translate("interval","interval"))
interval.default = 30
interval.optional = false
interval.rmempty = false


s = m:section(TypedSection, "downaddress", translate("downaddress","downaddress"))
s.template = "cbi/tblsection"
s.anonymous = false
s.addremove = true

enable = s:option(Flag, "addr_enable", translate("enable", "enable"))
enable.default = false
enable.optional = false
enable.rmempty = false


url = s:option(Value, "address", translate("dowload address","dowload address"))
url.optional = false
url.rmempty = false


return m

