local e
require"luci.util"
m=Map("devcom")
m.title=translate("设备名称备注")
s=m:section(TypedSection,"host",translate("Static Leases"),
translate("通过mac或ip为设备备注中文名，有mac则以mac为准，这里不能arp绑定，如需要，请移步DHCP/DNS" .. 
			"<br /><b><span style='color: red'>注意</span></b>：备注名不要填入空格"))
s.addremove=true
s.anonymous=true
s.template="cbi/tblsection"
comments=s:option(Value,"comments",translate("Comments"))
comments.rmempty=false
mac=s:option(Value,"mac",translate("<abbr title=\"Media Access Control\">MAC</abbr>-Address"))
mac.datatype="macaddr"
luci.sys.net.mac_hints(function(t,e)
if not luci.ip.new(e)then
mac:value(t,"%s (%s)"%{t,e})
end
end)
ip=s:option(Value,"ip",translate("<abbr title=\"Internet Protocol Version 4\">IPv4</abbr>-Address"))
ip.datatype="or(ip4addr,'ignore')"
luci.sys.net.ipv4_hints(function(t,e)
if not luci.ip.new(e)then
ip:value(t,"%s (%s)"%{t,e})
end
end)
wrt=s:option(Flag,"wrt",translate("<abbr title=\"为实时流量监测自定义主机信息，会覆盖其自身添加的文件\">WRT</abbr>"))
return m