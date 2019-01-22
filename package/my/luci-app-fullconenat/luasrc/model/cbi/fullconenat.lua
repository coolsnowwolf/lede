#-- Copyright (C) 2018 dz <dingzhong110@gmail.com>

require("nixio.fs")
require("luci.http")

m = Map("fullconenat", translate("Full cone NAT"),
	translate("FullConeNat."))

if luci.sys.call("iptables -t nat -L -n --line-numbers | grep FULLCONENAT >/dev/null") == 0 then
	m = Map("fullconenat", translate("FullConeNat"), "%s - %s" %{translate("FULLCONENAT"), translate("<strong><font color=\"green\">Running</font></strong>")})
else
	m = Map("fullconenat", translate("FullConeNat"), "%s - %s" %{translate("FULLCONENAT"), translate("<strong><font color=\"red\">Not Running</font></strong>")})
end


-- Basic
s = m:section(TypedSection, "fullconenat", translate("Settings"), translate("FullConeNat Settings"))
s.anonymous = true

enable = s:option(Flag, "enabled", translate("Enable"), translate("<strong><font color=\"red\">Warning!!! There is security risk after opening.</font></strong>"))
enable.default = 0
enable.rmempty = false

---- ALL Eanble
enable = s:option(Flag, "all_enabled", translate("ALL Enabled"))
enable.default = 0
enable.rmempty = false

--  fullconenat ip
o = s:option(Value, "fullconenat_ip", translate("FullConeNat IP"), translate("FullConeNat IP:192.168.1.100,192.168.1.101,192.168.1.102"))
o.rempty      = true

return m
