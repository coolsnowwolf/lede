
local m, s, o
local openclash = "openclash"
local uci = luci.model.uci.cursor()
local sys = require "luci.sys"
local sid = arg[1]
local fs = require "luci.openclash"

font_red = [[<font color="red">]]
font_off = [[</font>]]
bold_on  = [[<strong>]]
bold_off = [[</strong>]]

function IsYamlFile(e)
   e=e or""
   local e=string.lower(string.sub(e,-5,-1))
   return e == ".yaml"
end
function IsYmlFile(e)
   e=e or""
   local e=string.lower(string.sub(e,-4,-1))
   return e == ".yml"
end

m = Map(openclash, translate("Edit Proxy-Provider"))
m.pageaction = false
m.redirect = luci.dispatcher.build_url("admin/services/openclash/servers")
if m.uci:get(openclash, sid) ~= "proxy-provider" then
	luci.http.redirect(m.redirect)
	return
end

-- [[ Provider Setting ]]--
s = m:section(NamedSection, sid, "proxy-provider")
s.anonymous = true
s.addremove   = false

o = s:option(ListValue, "config", translate("Config File"))
o:value("all", translate("Use For All Config File"))
local e,a={}
for t,f in ipairs(fs.glob("/etc/openclash/config/*"))do
	a=fs.stat(f)
	if a then
    e[t]={}
    e[t].name=fs.basename(f)
    if IsYamlFile(e[t].name) or IsYmlFile(e[t].name) then
       o:value(e[t].name)
    end
  end
end

o = s:option(ListValue, "type", translate("Provider Type"))
o.rmempty = true
o.description = translate("Choose The Provider Type")
o:value("http")
o:value("file")

o = s:option(Value, "name", translate("Provider Name"))
o.rmempty = false

o = s:option(ListValue, "path", translate("Provider Path"))
o.description = translate("Update Your Proxy Provider File From Config Luci Page")
local p,h={}
for t,f in ipairs(fs.glob("/etc/openclash/proxy_provider/*"))do
	h=fs.stat(f)
	if h then
    p[t]={}
    p[t].name=fs.basename(f)
    if IsYamlFile(p[t].name) or IsYmlFile(p[t].name) then
       o:value("./proxy_provider/"..p[t].name)
    end
  end
end
o.rmempty = false
o:depends("type", "file")

o = s:option(Value, "provider_url", translate("Provider URL"))
o.rmempty = false
o:depends("type", "http")

o = s:option(Value, "provider_interval", translate("Provider Interval(s)"))
o.default = "3600"
o.rmempty = false
o:depends("type", "http")

o = s:option(ListValue, "health_check", translate("Provider Health Check"))
o:value("false", translate("Disable"))
o:value("true", translate("Enable"))
o.default=true

o = s:option(Value, "health_check_url", translate("Health Check URL"))
o:value("http://www.gstatic.com/generate_204")
o:value("https://cp.cloudflare.com/generate_204")
o.rmempty = false

o = s:option(Value, "health_check_interval", translate("Health Check Interval(s)"))
o.default = "300"
o.rmempty = false

o = s:option(DynamicList, "groups", translate("Proxy Group"))
o.description = font_red..bold_on..translate("No Need Set when Config Create, The added Proxy Groups Must Exist")..bold_off..font_off
o.rmempty = true
m.uci:foreach("openclash", "groups",
		function(s)
			if s.name ~= "" and s.name ~= nil then
			   o:value(s.name)
			end
		end)

local t = {
    {Commit, Back}
}
a = m:section(Table, t)

o = a:option(Button,"Commit")
o.inputtitle = translate("Commit Configurations")
o.inputstyle = "apply"
o.write = function()
   m.uci:commit(openclash)
   luci.http.redirect(m.redirect)
end

o = a:option(Button,"Back")
o.inputtitle = translate("Back Configurations")
o.inputstyle = "reset"
o.write = function()
   m.uci:revert(openclash)
   luci.http.redirect(m.redirect)
end

return m
