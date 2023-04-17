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
m.pageaction = false
s.anonymous = true
s.addremove=false

y = s:option(ListValue, "dnsforwader", translate("DNS Forwarding"))
y:value("0", translate("disabled"))
y:value("1", translate("enabled"))
y.description = translate("Set custom DNS forwarder in DHCP and DNS Settings and forward all dns traffic to clash")


y = s:option(ListValue, "dnscache", translate("DNS Cache"))
y:value("0", translate("disabled"))
y:value("1", translate("enabled"))
y.description = translate("Set to enable or disable dns cache")

y = s:option(ListValue, "access_control", translate("Access Control"))
y:value("0", translate("disabled"))
y:value("1", translate("Whitelist IPs"))
y:value("2", translate("Blacklist Ips"))
y.description = translate("Whitelist or Blacklist IPs to use Clash ( Only support enhanced-mode: redir-host )")

o = s:option(DynamicList, "proxy_lan_ips", translate("Proxy Lan List"))
o.datatype = "ipaddr"
o.description = translate("Only selected IPs will be proxied")
luci.ip.neighbors({ family = 4 }, function(entry)
       if entry.reachable then
               o:value(entry.dest:string())
       end
end)
o:depends("access_control", 1)


o = s:option(DynamicList, "reject_lan_ips", translate("Bypass Lan List"))
o.datatype = "ipaddr"
o.description = translate("Selected IPs will not be proxied")
luci.ip.neighbors({ family = 4 }, function(entry)
       if entry.reachable then
               o:value(entry.dest:string())
       end
end)
o:depends("access_control", 2)


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
  	luci.http.redirect(luci.dispatcher.build_url("admin", "services", "clash" , "settings", "dns", "advance"))
end
end

return m