
local m, s, o
local openclash = "openclash"
local uci = luci.model.uci.cursor()
local fs = require "luci.openclash"
local sys = require "luci.sys"
local sid = arg[1]

font_red = [[<b style=color:red>]]
font_off = [[</b>]]
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

m = Map(openclash, translate("Edit Rule Providers"))
m.pageaction = false
m.description=translate("规则集使用介绍：https://lancellc.gitbook.io/clash/clash-config-file/rule-provider")
m.redirect = luci.dispatcher.build_url("admin/services/openclash/rule-providers-settings")
if m.uci:get(openclash, sid) ~= "rule_providers" then
	luci.http.redirect(m.redirect)
	return
end

-- [[ Rule Providers Setting ]]--
s = m:section(NamedSection, sid, "rule_providers")
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

o = s:option(Value, "name", translate("Rule Providers Name"))
o.rmempty = false
o.default = "Rule-provider - "..sid

o = s:option(ListValue, "type", translate("Rule Providers Type"))
o.rmempty = true
o.description = translate("Choose The Rule Providers Type")
o:value("http", translate("http"))
o:value("file", translate("file"))

o = s:option(ListValue, "behavior", translate("Rule Behavior"))
o.rmempty = true
o.description = translate("Choose The Rule Behavior")
o:value("domain")
o:value("ipcidr")
o:value("classical")

o = s:option(ListValue, "format", translate("Rule Format")..translate("(TUN&Meta Core)"))
o.rmempty = true
o.description = translate("Choose The Rule File Format, For More Info:").." ".."<a href='javascript:void(0)' onclick='javascript:return winOpen(\"https://github.com/Dreamacro/clash/wiki/Premium%3A-Rule-Providers\")'>https://github.com/Dreamacro/clash/wiki/</a>"
o:value("yaml")
o:value("text")

o = s:option(ListValue, "path", translate("Rule Providers Path"))
o.description = translate("Update Your Rule Providers File From Config Luci Page")
local p,h={}
for t,f in ipairs(fs.glob("/etc/openclash/rule_provider/*"))do
	h=fs.stat(f)
	if h then
    p[t]={}
    p[t].name=fs.basename(f)
    o:value("./rule_provider/"..p[t].name)
  end
end
for t,f in ipairs(fs.glob("/etc/openclash/game_rules/*"))do
	h=fs.stat(f)
	if h then
    p[t]={}
    p[t].name=fs.basename(f)
    o:value("./game_rules/"..p[t].name)
  end
end
o.rmempty = false
o:depends("type", "file")

o = s:option(Value, "url", translate("Rule Providers URL"))
o.rmempty = false
o:depends("type", "http")

o = s:option(Value, "interval", translate("Rule Providers Interval(s)"))
o.default = "86400"
o.rmempty = false
o:depends("type", "http")

o = s:option(ListValue, "position", translate("Append Position"))
o.rmempty     = false
o:value("0", translate("Priority Match"))
o:value("1", translate("Extended Match"))

o = s:option(ListValue, "group", translate("Set Proxy Group"))
o.description = font_red..bold_on..translate("The Added Proxy Groups Must Exist Except 'DIRECT' & 'REJECT'")..bold_off..font_off
o.rmempty = true
local groupnames,filename
filename = m.uci:get(openclash, "config", "config_path")
if filename then
   groupnames = sys.exec(string.format('ruby -ryaml -rYAML -I "/usr/share/openclash" -E UTF-8 -e "YAML.load_file(\'%s\')[\'proxy-groups\'].each do |i| puts i[\'name\']+\'##\' end" 2>/dev/null',filename))
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

local t = {
    {Commit, Back}
}
a = m:section(Table, t)

o = a:option(Button,"Commit", " ")
o.inputtitle = translate("Commit Settings")
o.inputstyle = "apply"
o.write = function()
   m.uci:commit(openclash)
   sys.call("/usr/share/openclash/yml_groups_name_ch.sh")
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
