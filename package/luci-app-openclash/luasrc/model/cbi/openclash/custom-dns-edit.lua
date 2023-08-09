local m, s, o
local openclash = "openclash"
local uci = luci.model.uci.cursor()
local fs = require "luci.openclash"
local SYS = require "luci.sys"
local sid = arg[1]

font_red = [[<b style=color:red>]]
font_green = [[<b style=color:green>]]
font_off = [[</b>]]
bold_on  = [[<strong>]]
bold_off = [[</strong>]]

m = Map(openclash, translate("Add Custom DNS Servers"))
m.pageaction = false
m.redirect = luci.dispatcher.build_url("admin/services/openclash/config-overwrite")
if m.uci:get(openclash, sid) ~= "dns_servers" then
	luci.http.redirect(m.redirect)
	return
end

-- [[ Edit Custom DNS ]] --
s = m:section(NamedSection, sid, "dns_servers")
s.anonymous = true
s.addremove   = false

---- group
o = s:option(ListValue, "group", translate("DNS Server Group"))
o.description = font_red..bold_on..translate("NameServer Group Must Be Set")..bold_off..font_off
o:value("nameserver", translate("NameServer "))
o:value("fallback", translate("FallBack "))
o:value("default", translate("Default-NameServer"))
o.default     = "nameserver"
o.rempty      = false

---- IP address
o = s:option(Value, "ip", translate("DNS Server Address"))
o.description = translate("Do Not Add Type Ahead")
o.placeholder = translate("Not Null")
o.datatype = "or(host, string)"
o.rmempty = true

---- port
o = s:option(Value, "port", translate("DNS Server Port"))
o.description = translate("Require When Use Non-Standard Port")
o.datatype    = "port"
o.rempty      = true

---- type
o = s:option(ListValue, "type", translate("DNS Server Type"))
o.description = translate("Communication protocol")
o:value("udp", translate("UDP"))
o:value("tcp", translate("TCP"))
o:value("tls", translate("TLS"))
o:value("https", translate("HTTPS"))
o:value("quic", translate("QUIC ")..translate("(Only Meta Core)"))
o.default     = "udp"
o.rempty      = false

---- interface
o = s:option(Value, "interface", translate("Specific Interface"))
o.description = translate("DNS Lookup Only Through The Specific Interface")..translate("(Only TUN Core)")
local interfaces = SYS.exec("ls -l /sys/class/net/ 2>/dev/null |awk '{print $9}' 2>/dev/null")
for interface in string.gmatch(interfaces, "%S+") do
	o:value(interface)
end
o:value("Disable", translate("Disable"))
o.default = "Disable"
o.rempty = false

---- Node Domain Resolve
o = s:option(Flag, "node_resolve", translate("Node Domain Resolve"), translate("Use For Node Domain Resolve")..translate("(Only Meta Core)"))
o.rmempty     = false
o.default     = o.disbled

---- Force HTTP/3
o = s:option(Flag, "http3", translate("Force HTTP/3"), translate("Force HTTP/3 to connect")..translate("(Only Meta Core)"))
o:depends("type", "https")
o.rmempty     = false
o.default     = o.disbled

---- Proxy group
o = s:option(Value, "specific_group", translate("Specific Group"))
o.description = translate("Group Use For Proxy The DNS")..translate("(Only Meta Core)")
o:depends("group", "nameserver")
o:depends("group", "fallback")
local groupnames,filename
filename = m.uci:get(openclash, "config", "config_path")
if filename then
	groupnames = SYS.exec(string.format('ruby -ryaml -rYAML -I "/usr/share/openclash" -E UTF-8 -e "YAML.load_file(\'%s\')[\'proxy-groups\'].each do |i| puts i[\'name\']+\'##\' end" 2>/dev/null',filename))
	if groupnames then
		for groupname in string.gmatch(groupnames, "([^'##\n']+)##") do
			if groupname ~= nil and groupname ~= "" then
				o:value(groupname)
			end
		end
	end
end

m.uci:foreach("openclash", "groups",
function(s)
	if s.name ~= "" and s.name ~= nil then
		o:value(s.name)
	end
end)
   
o:value("DIRECT")
o:value("REJECT")
o:value("Disable", translate("Disable"))
o.default = "Disable"
o.rempty = false

local t = {
    {Commit, Back}
}
a = m:section(Table, t)

o = a:option(Button,"Commit", " ")
o.inputtitle = translate("Commit Settings")
o.inputstyle = "apply"
o.write = function()
   m.uci:commit(openclash)
   luci.http.redirect(m.redirect)
end

o = a:option(Button,"Back", " ")
o.inputtitle = translate("Back Settings")
o.inputstyle = "reset"
o.write = function()
   m.uci:revert(openclash, sid)
   luci.http.redirect(m.redirect)
end

m:append(Template("openclash/toolbar_show"))
return m
