
mp = Map("ipsec", translate("IPSec VPN Server"))
mp.description = translate("IPSec VPN connectivity using the native built-in VPN Client on iOS or Andriod (IKEv1 with PSK and Xauth)")

mp:section(SimpleSection).template  = "ipsec/ipsec_status"

s = mp:section(NamedSection, "ipsec", "service")
s.anonymouse = true

enabled = s:option(Flag, "enabled", translate("Enable"))
enabled.default = 0
enabled.rmempty = false

clientip = s:option(Value, "clientip", translate("VPN Client IP"))
clientip.datatype = "ip4addr"
clientip.description = translate("LAN DHCP reserved started IP addresses with the same subnet mask")
clientip.optional = false
clientip.rmempty = false

clientdns = s:option(Value, "clientdns", translate("VPN Client DNS"))
clientdns.datatype = "ip4addr"
clientdns.description = translate("DNS using in VPN tunnel.Set to the router's LAN IP is recommended")
clientdns.optional = false
clientdns.rmempty = false

account = s:option(Value, "account", translate("Account"))
account.datatype = "string"

password = s:option(Value, "password", translate("Password"))
password.password = true

secret = s:option(Value, "secret", translate("Secret Pre-Shared Key"))
secret.password = true

return mp
