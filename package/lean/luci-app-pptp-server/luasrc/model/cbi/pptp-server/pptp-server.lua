
mp = Map("pptpd", translate("PPTP VPN Server"))
mp.description = translate("PPTP VPN Server connectivity using the native built-in VPN Client on Windows/Linux or Andriod")

mp:section(SimpleSection).template  = "pptp/pptp_status"

s = mp:section(NamedSection, "pptpd", "service")
s.anonymouse = true

enabled = s:option(Flag, "enabled", translate("Enable"))
enabled.default = 0
enabled.rmempty = false

localip = s:option(Value, "localip", translate("Local IP"))
localip.datatype = "ip4addr"

clientip = s:option(Value, "remoteip", translate("Client IP"))
clientip.datatype = "string"
clientip.description = translate("LAN DHCP reserved start-to-end IP addresses with the same subnet mask")

remotedns = s:option(Value, "remotedns", translate("Remote Client DNS"))
remotedns.datatype = "ip4addr"

logging = s:option(Flag, "logwtmp", translate("Debug Logging"))
logging.default = 0
logging.rmempty = false

logins = mp:section(NamedSection, "login", "login", translate("PPTP Logins"))
logins.anonymouse = true

username = logins:option(Value, "username", translate("User name"))
username.datatype = "string"

password = logins:option(Value, "password", translate("Password"))
password.password = true

return mp
