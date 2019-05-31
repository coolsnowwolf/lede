local e=require"nixio.fs"
local e=luci.http
local o=require"luci.model.network".init()
local a,t,e,b

--softethervpn是配置文件
a=Map("softethervpn",translate("Softether VPN Server"),translate("Softether VPN Server connectivity using the native built-in VPN Client on Windows/Linux or Andriod"))
a:section(SimpleSection).template  = "softether/status"

t=a:section(NamedSection,"setting_enabled","softether")
t.anonymous=true
t.addremove=false

t:tab("basic", translate("Base Setting"))

e=t:taboption("basic", Flag, "enabled", translate("VPN Server Enabled"))
e.default=0
e.rmempty=false

return a
