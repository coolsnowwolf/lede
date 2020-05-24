local o
require "luci.util"
m=Map("devcom")
m.title=translate("设备名称备注")
s=m:section(TypedSection,"host",translate("Static Leases"),
translate("通过mac或ip为设备备注中文名，有mac则以mac为准，这里不能arp绑定，如需要，请移步DHCP/DNS"))
s.addremove=true
s.anonymous=true
s.template="cbi/tblsection"
comments=s:option(Value,"comments",translate("Comments"))
comments.rmempty=false
mac=s:option(Value,"mac",translate("<abbr title=\"Media Access Control\">MAC</abbr>-Address"))
mac.datatype = "macaddr"
luci.sys.net.mac_hints(function(x,d)
	if not luci.ip.new(d) then
		mac:value(x,"%s (%s)"%{x,d})
	end
end)
ip=s:option(Value,"ip",translate("<abbr title=\"Internet Protocol Version 4\">IPv4</abbr>-Address"))
ip.datatype="or(ip4addr,'ignore')"
luci.sys.net.ipv4_hints(function(x,d)
	if not luci.ip.new(d) then
		ip:value(x,"%s (%s)"%{x,d})
	end
end)

return m