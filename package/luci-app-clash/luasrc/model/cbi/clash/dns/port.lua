local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local uci = require("luci.model.uci").cursor()

m = Map("clash")
s = m:section(TypedSection, "clash")
m.pageaction = false
s.anonymous = true

o = s:option(Value, "http_port")
o.title = translate("Http Port")
o.default = 7890
o.datatype = "port"
o.rmempty = false
o.description = translate("Http Port")

o = s:option(Value, "socks_port")
o.title = translate("Socks Port")
o.default = 7891
o.datatype = "port"
o.rmempty = false
o.description = translate("Socks Port")

o = s:option(Value, "redir_port")
o.title = translate("Redir Port")
o.default = 7892
o.datatype = "port"
o.rmempty = false
o.description = translate("Redir Port")

o = s:option(Value, "mixed_port")
o.title = translate("Mixed Port")
o.default = 7893
o.datatype = "port"
o.rmempty = false
o.description = translate("Mixed Port")

o = s:option(ListValue, "allow_lan")
o.title = translate("Allow Lan")
o.default = true
o.rmempty = false
o:value("true", "true")
o:value("false", "false")
o.description = translate("Allow Lan")

o = s:option(ListValue, "enable_ipv6")
o.title = translate("Enable IPv6")
o.default = false
o.rmempty = false
o:value("true", "true")
o:value("false", "false")
o.description = translate("Enable IPv6")

o = s:option(Value, "bind_addr")
o.title = translate("Bind Address")
o:value("*",  translate("Bind All IP Addresses"))
luci.ip.neighbors({ family = 4 }, function(entry)
       if entry.reachable then
               o:value(entry.dest:string())
       end
end)
luci.ip.neighbors({ family = 6 }, function(entry)
       if entry.reachable then
               o:value(entry.dest:string())
       end
end)
o.description = translate("Bind Address")
o:depends("allow_lan", "true")


o = s:option(Value, "dash_port")
o.title = translate("Dashboard Port")
o.default = 9090
o.datatype = "port"
o.rmempty = false
o.description = translate("Dashboard Port")

o = s:option(Value, "dash_pass")
o.title = translate("Dashboard Secret")
o.default = 123456
o.rmempty = false
o.description = translate("Dashboard Secret")

o = s:option(ListValue, "p_mode", translate("Proxy Mode"))
o.description = translate("Choose proxy mode")
o:value("rule", translate("RULE"))
o:value("global", translate("GLOBAL"))
o:value("Script", translate("SCRIPT"))
o:value("direct", translate("DIRECT"))
o.default = "Rule"

o = s:option(ListValue, "level", translate("Log level"))
o.description = translate("Choose Log Level")
o:value("info", "info")
o:value("silent", "silent")
o:value("warning", "warning")
o:value("error", "error")
o:value("debug", "debug")

o = s:option(Button, "Apply")
o.title = luci.util.pcdata(translate("Save & Apply"))
o.inputtitle = translate("Save & Apply")
o.inputstyle = "apply"
o.write = function()
m.uci:commit("clash")
if luci.sys.call("pidof clash >/dev/null") == 0 then
	SYS.call("/etc/init.d/clash restart >/dev/null 2>&1 &")
        luci.http.redirect(luci.dispatcher.build_url("admin", "services", "clash"))
else
  	luci.http.redirect(luci.dispatcher.build_url("admin", "services", "clash" , "settings", "port"))
end
end

return m
