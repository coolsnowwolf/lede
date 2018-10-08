--[[

LuCI mentohust
Author:ptpt52
Email:ptpt52@gmail.com

]]--

require("luci.tools.webadmin")

m = Map("mentohust", translate("MentoHUST"), translate("A Ruijie and Cernet supplicant on Linux and MacOS"))

m:section(SimpleSection).template  = "mentohust/mentohust"

s = m:section(TypedSection, "option", translate("Start option"),translate("Configure mentohust's start option"))
s.anonymous = true
s:option(Flag, "enable", translate("MentoHUST_enable"), translate("enable or disable mentohust")).default="0"
s:option(Flag, "rproxy", translate("Router Proxy"), translate("router proxy to fake single client")).default="0"
s:option(Value, "nf_limit", translate("max conntrack limits"),translate("0 means no limit, default to 280")).default="280"

s = m:section(TypedSection, "mentohust", translate("Config mentohust"),translate("The options below are all of mentohust's arguments."))
s.anonymous = true
s:option(Value, "Username", translate("Username")).default="yourusername"

pw=s:option(Value, "Password", translate("Password"))
pw.password = true
pw.rmempty = false
pw.default= "yourpassword"

nic=s:option(ListValue, "Nic", translate("net card name"))
nic.anonymous = true
for k, v in pairs(luci.sys.net.devices()) do
	nic:value(v)
end

s:option(Value, "ServiceName", translate("ServiceName"),translate("default to 'internet'")).default="internet"

s:option(Value, "IP", translate("IP"),translate("default to localhost's IP")).default="0.0.0.0"

s:option(Value, "Mask", translate("netmask"),translate("default to localhost's netmask")).default="255.255.255.0"

s:option(Value, "Gateway", translate("gateway"),translate("default to 0.0.0.0")).default="0.0.0.0"

s:option(Value, "DNS", translate("DNS"),translate("default to 0.0.0.0")).default="0.0.0.0"

s:option(Value, "PingHost", translate("Ping host"),translate("default to 0.0.0.0,i.e. disable this function")).default="0.0.0.0"

s:option(Value, "Timeout", translate("authenticate timeout(s)"),translate("default to 8s")).default="8"

s:option(Value, "EchoInterval", translate("heartbeat timeout(s)"),translate("default to 30s")).default="30"

s:option(Value, "RestartWait", translate("wait on failure timeout(s)"),translate("default to 15s")).default="15"

s:option(Value, "MaxFail", translate("max failure times"),translate("0 means no limit,default to 8")).default="8"

t=s:option(ListValue, "StartMode", translate("mulcast address"),translate("default to 0"))
t:value("0", translate("0(standard)"))
t:value("1", translate("1(ruijjie)"))
t:value("2", translate("2(saier)"))
t.default = "0"

t= s:option(ListValue, "DhcpMode", translate("DHCP type"),translate("default to 0"))
t:value("0", translate("0(not in used)"))
t:value("1", translate("1(secondary authenticate)"))
t:value("2", translate("2(post authenticate)"))
t:value("3", translate("3(pre authenticate)"))
t.default = "0"

s:option(Value, "Version", translate("client version"),translate("default to 0.00,compatible with xrgsu")).default="0.00"

s:option(Value, "DataFile", translate("customized data file"),translate("not in used by default")).default="/etc/mentohust"

s:option(Value, "DhcpScript", translate("DHCP script"),translate("use dhclient by default")).default="udhcpc -i"

return m
