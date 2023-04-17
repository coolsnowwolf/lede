local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local uci = require("luci.model.uci").cursor()
local clash = "clash"
local http = luci.http


m = Map("clash")
s = m:section(TypedSection, "clash")
--m.pageaction = false
s.anonymous = true
s.addremove=false


y = s:option(Flag, "interf", translate("Interface Name"))
y.default = 1
y.description = translate("Enable Interface Name")

y = s:option(Value, "interf_name", translate("Set Interface Name"))
y.description = translate("Set Interface Name")
y:depends("interf", 1)

y = s:option(Flag, "tun_mode", translate("Tun Mode"))
y.default = 0
y.description = translate("Enable Tun and make sure you are using tun supported core")

y = s:option(ListValue, "stack", translate("Stack"))
y:value("system", translate("system"))
y:value("gvisor", translate("gvisor"))
y.description = translate("Select Stack Mode")
y:depends("tun_mode", 1)

y = s:option(Flag, "enable_dns", translate("Enable Clash DNS"))
y.default = 1
y.description = translate("Enable Clash DNS")

y = s:option(Value, "listen_port", translate("DNS Listen Port"))
y.description = translate("Set DNS Listen Port")
y.default  = "5300"


y = s:option(DynamicList, "default_nameserver", translate("Default NameServer"))
y.description = translate("Default Nameserver List")


y = s:option(ListValue, "enhanced_mode", translate("Enhanced Mode"))
y:value("redir-host", translate("Redir Host"))
y:value("fake-ip", translate("Fake IP"))
y.description = translate("Select Enhanced Mode")

y = s:option(Value, "fake_ip_range", translate("Fake IP Range"))
y.description = translate("Set Fake IP Range")
y.default  = "198.18.0.1/16"
y:depends("enhanced_mode", "fake-ip")

y = s:option(DynamicList, "fake_ip_filter", translate("Fake IP Filter"))
y.description = translate("Fake IP Filter List")
y.default  = "*.lan"
y:depends("enhanced_mode", "fake-ip")

s = m:section(TypedSection, "dnshijack", translate("DNS Hijack"))
s.anonymous = true
s.addremove = true
s.sortable = false
s.template = "cbi/tblsection"
s.rmempty = false

o = s:option(Flag, "enabled", translate("Enable"))
o.rmempty     = false
o.default     = o.enabled
o.cfgvalue    = function(...)
    return Flag.cfgvalue(...) or "1"
end

o = s:option(ListValue, "type", translate("Protocol"))
o:value("none", translate("No Protocol"))
o:value("tcp://", translate("TCP"))
o:value("udp://", translate("UDP"))
o:value("tls://", translate("TLS"))
o:value("https://", translate("HTTPS"))
o.default     = "none"
o.rempty      = false

o = s:option(Value, "ip", translate("Address"))
o.placeholder = translate("Not Null")
o.datatype = "or(host, string)"
o.rmempty = true

o = s:option(Value, "port", translate("Port"))
o.datatype    = "port"
o.rempty      = true


s = m:section(TypedSection, "authentication", translate("Authentication"))
s.anonymous = true
s.addremove = true
s.sortable = false
s.template = "cbi/tblsection"
s.rmempty = false

o = s:option(Flag, "enabled", translate("Enable"))
o.rmempty     = false
o.default     = o.enabled
o.cfgvalue    = function(...)
    return Flag.cfgvalue(...) or "1"
end

o = s:option(Value, "username", translate("Username"))
o.placeholder = translate("Cannot be Empty")
o.rempty      = true

o = s:option(Value, "password", translate("Password"))
o.placeholder = translate("Cannot be Empty")
o.rmempty = true



s = m:section(TypedSection, "hosts", translate("Hosts"))
s.anonymous = true
s.addremove = true
s.sortable = false
s.template = "cbi/tblsection"
s.rmempty = false

o = s:option(Flag, "enabled", translate("Enable"))
o.rmempty     = false
o.default     = o.enabled
o.cfgvalue    = function(...)
    return Flag.cfgvalue(...) or "1"
end

o = s:option(Value, "address", translate("Address"))
o.placeholder = translate("Cannot be Empty")
o.rempty      = true

o = s:option(Value, "ip", translate("IP"))
o.placeholder = translate("Cannot be Empty")
o.rmempty = true



s = m:section(TypedSection, "dnsservers", translate("Nameservers & Fallback"))
s.anonymous = true
s.addremove = true
s.sortable = false
s.template = "cbi/tblsection"
s.rmempty = false

o = s:option(Flag, "enabled", translate("Enable"))
o.rmempty     = false
o.default     = o.enabled
o.cfgvalue    = function(...)
    return Flag.cfgvalue(...) or "1"
end

o = s:option(ListValue, "ser_type", translate("Type"))
o:value("nameserver", translate("NameServer"))
o:value("fallback", translate("FallBack"))

o = s:option(ListValue, "protocol", translate("Protocol"))
o:value("none", translate("No Protocol"))
o:value("tcp://", translate("TCP"))
o:value("udp://", translate("UDP"))
o:value("tls://", translate("TLS"))
o:value("https://", translate("HTTPS"))

o = s:option(Value, "ser_address", translate("Address"))
o.placeholder = translate("Not Empty")
o.rmempty = false

o = s:option(Value, "ser_port", translate("Port"))


local apply = luci.http.formvalue("cbi.apply")
if apply then
  m.uci:commit("clash")
if luci.sys.call("pidof clash >/dev/null") == 0 then
	SYS.call("/etc/init.d/clash restart >/dev/null 2>&1 &")
	luci.http.redirect(luci.dispatcher.build_url("admin", "services", "clash"))
end  
end



return m

