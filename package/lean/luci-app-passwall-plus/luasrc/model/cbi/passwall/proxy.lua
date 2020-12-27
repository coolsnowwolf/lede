m = Map("passwall", translate("Proxy Server Forward List"))

s = m:section(TypedSection, "proxy")
s.anonymous = true
s.addremove = true

s:tab("forward", translate("Server"))

o = s:taboption("forward", Value, "name", translate("Name"))
o.datatype = "host"

o = s:taboption("forward", ListValue, "proxy", translate("Proxy Type"))
o:value("direct", translate("direct"))
o:value("http", translate("http"))
o:value("https", translate("https"))
o:value("socks4", translate("socks4"))
o:value("socks4a", translate("socks4a"))
o:value("socks5", translate("socks5"))
o.default = "direct"
o.rmempty = false

o = s:taboption("forward", Value, "address", translate("IP"))
o.datatype = "ip4addr"

o = s:taboption("forward", Value, "port", translate("Port"))
o.datatype = "port"

s:tab("authentication", translate("Authentication"))

o = s:taboption("authentication", Value, "user", translate("User"))
o.datatype = "string"

o = s:taboption("authentication", Value, "password", translate("Password"))
o.datatype = "string"

s:tab("settings", translate("Settings"))
o = s:taboption("settings", Flag, "resolve", translate("DnsResolve"))
o.datatype = "enable"
o.description = translate("Whether to perform local DNS resolution, only direct connection and SOCKS4 are recommended")

o = s:taboption("settings", Value, "sleep", translate("Sleep"))
o.datatype = "max(30000)"
o.default = 160
o.description = translate("Time to wait before using this line (in milliseconds)")

o = s:taboption("settings", Value, "delay", translate("CorrectDelay"))
o.datatype = "max(350)"
o.description = translate("The non-SS protocol does not need to be set. The SS protocol is recommended to be set to 50-100")


s:tab("white", translate("WhiteList"))
o = s:taboption("white", DynamicList, "white", translate("Domain"))
o.datatype = "hostname"

return m
