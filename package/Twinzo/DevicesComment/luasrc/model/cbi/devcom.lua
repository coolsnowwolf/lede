local ipc = require "luci.ip"
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
mac.datatype="list(macaddr)"
luci.ip.neighbors({family = 4}, function(neighbor)
	if neighbor.reachable then
		mac:value(neighbor.mac, "%s (%s)" %{neighbor.mac, neighbor.dest:string()})
	end
end)
ip=s:option(Value,"ip",translate("<abbr title=\"Internet Protocol Version 4\">IPv4</abbr>-Address"))
ip.datatype="or(ip4addr,'ignore')"
luci.ip.neighbors({ family = 4 }, function(entry)
       if entry.reachable then
               ip:value(entry.dest:string())
       end
end)

return m