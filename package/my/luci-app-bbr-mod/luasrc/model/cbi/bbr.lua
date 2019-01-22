#-- Copyright (C) 2018 dz <dingzhong110@gmail.com>

require("nixio.fs")
require("luci.http")

m = Map("bbr", translate("BBR"),
	translate("BBR."))

if luci.sys.call("sysctl net.ipv4.tcp_congestion_control | grep bbr >/dev/null") == 0 then
	m = Map("bbr", translate("bbr"), "%s - %s" %{translate("bbr"), translate("<strong><font color=\"green\">Running</font></strong>")})
else
	m = Map("bbr", translate("bbr"), "%s - %s" %{translate("bbr"), translate("<strong><font color=\"red\">Not Running</font></strong>")})
end


-- Basic
s = m:section(TypedSection, "bbr", translate("Settings"), translate("bbr Settings"))
s.anonymous = true

enable = s:option(Flag, "enabled", translate("Enable"), translate("Enable Too"))
enable.default = 0
enable.rmempty = false

---- ALL Eanble
enable = s:option(Flag, "bbr_mod_enabled", translate("BBR Mod Enabled"))
enable.default = 0
enable.rmempty = false

return m

