-- Copyright (C) 2018 max0y <askmaxwork@gmail.com>
-- Licensed to the public under the GNU General Public License v3.

require("luci.sys")

local eap_list = {
	"MD5",
	"PEAP",
	"MSCHAPV2",
}

m = Map("cd8021x", translate("802.1x Client"), 
    translate("Configure IEEE 802.1x wired authentication, you may need to edit your WAN interface protocol as <i>DHCP</i> client <a href=\"network\">here</a>"))

s = m:section(TypedSection, "login", "")
s.addremove = false
s.anonymous = true

enable = s:option(Flag, "enable", translate("Enable"))
name = s:option(Value, "username", translate("Username"))
pass = s:option(Value, "password", translate("Password"))
pass.password = true

ifname = s:option(ListValue, "ifname", translate("Interfaces"))
for k, v in ipairs(luci.sys.net.devices()) do
    if v ~= "lo" then
        ifname:value(v)
    end
end

eap = s:option(ListValue, "eap", translate("EAP"))
for k, v in ipairs(eap_list) do
    eap:value(v)
end

local apply = luci.http.formvalue("cbi.apply")
if apply then
    io.popen("/etc/init.d/cd8021x restart")
end

return m
