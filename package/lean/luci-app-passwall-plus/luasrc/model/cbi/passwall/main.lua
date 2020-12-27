-- 第一个为配置文件 默认路径是"/etc/config"+passwall
-- m = Map("passwall", translate("PassWall Plus"), translate("On this page we can learn how the .lua work"))
m = Map("passwall", translate("PassWall Plus"))
-- 第二个为配置文件具体项 global
-- s = m:section(TypedSection, "global", translate("Script Configuration"))

m:section(SimpleSection).template = "passwall/status"

s = m:section(TypedSection, "global")
s.anonymous = true
s.addremove = false

s:tab("main", translate("Settings"))

o = s:taboption("main", Flag, "switch", translate("enable"))
o.datatype = "enable"

o = s:taboption("main", DynamicList, "port", translate("http port"))
o.datatype = "port"
o.description = translate("Ports that need to redirect http traffic")

o = s:taboption("main", DynamicList, "ports", translate("https port"))
o.datatype = "port"
o.description = translate("Ports that need to redirect https traffic")

s:tab("dns", translate("DNS"))

o = s:taboption("dns", Flag, "ipv6", translate("IPV6"))
o.datatype = "enable"
o.description = translate("ipv6 address return")

o = s:taboption("dns", Value, "cache", translate("dns cache size"))
o.datatype = "max(4096)"

o = s:taboption("dns", DynamicList, "server", translate("remote udp dns server"))
o.datatype = "string"
o.placeholder = luci.sys.exec("head -n1 /tmp/resolv.conf.ppp|cut -d' ' -f2")..":53"
o.description = translate("examples: 223.5.5.5:53 or 127.0.0.1:1053")

o = s:taboption("dns", DynamicList, "bogus", translate("bogus NX domain results"))
o.datatype = "string"
o.description = translate("Filter out contaminated return IP addresses. examples: 127.0.0.1/8 or 1.2.3.4")

s:tab("block", translate("Black Domain List"))
local blockconf = "/etc/black.list"
o = s:taboption("block", TextValue, "blockconf")
o.rows = 13
o.wrap = "off"
o.rmempty = true
o.cfgvalue = function(self, section)
	return nixio.fs.readfile(blockconf) or " "
end
o.write = function(self, section, value)
	nixio.fs.writefile(blockconf, value:gsub("\r\n", "\n"))
end
o.remove = function(self, section, value)
	nixio.fs.writefile(blockconf, "")
end


return m
