
local m, s, o
local openclash = "openclash"
local uci = luci.model.uci.cursor()
local fs = require "luci.openclash"
local sys = require "luci.sys"
local sid = arg[1]

font_red = [[<font color="red">]]
font_green = [[<font color="green">]]
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

m = Map(openclash, translate("Other Rules Edit"))
m.pageaction = false
m.redirect = luci.dispatcher.build_url("admin/services/openclash/settings")
if m.uci:get(openclash, sid) ~= "other_rules" then
	luci.http.redirect(m.redirect)
	return
end

-- [[ Other Rules Setting ]]--
s = m:section(NamedSection, sid, "other_rules")
s.anonymous = true
s.addremove   = false

o = s:option(ListValue, "config", translate("Config File"))
local e,a={}
local groupnames,filename
for t,f in ipairs(fs.glob("/etc/openclash/config/*"))do
	a=fs.stat(f)
	if a then
    e[t]={}
    e[t].name=fs.basename(f)
    if IsYamlFile(e[t].name) or IsYmlFile(e[t].name) then
       o:value(e[t].name)
    end
    if e[t].name == m.uci:get(openclash, sid, "config") then
    	filename = e[t].name
      groupnames = sys.exec(string.format('ruby -ryaml -E UTF-8 -e "YAML.load_file(\'%s\')[\'proxy-groups\'].each do |i| puts i[\'name\']+\'##\' end" 2>/dev/null',f))
    end
  end
end

o = s:option(Button, translate("Get Group Names")) 
o.title = translate("Get Group Names")
o.inputtitle = translate("Get Group Names")
o.description = translate("Get Group Names After Select Config File")
o.inputstyle = "reload"
o.write = function()
  m.uci:commit("openclash")
  luci.http.redirect(luci.dispatcher.build_url("admin/services/openclash/other-rules-edit/%s") % sid)
end

if groupnames ~= nil and filename ~= nil then
o = s:option(ListValue, "rule_name", translate("Other Rules Name"))
o.rmempty = true
o:value("lhie1", translate("lhie1 Rules"))
o:value("ConnersHua", translate("ConnersHua(Provider-type) Rules"))
o:value("ConnersHua_return", translate("ConnersHua Return Rules"))

o = s:option(ListValue, "GlobalTV", translate("GlobalTV"))
o:depends("rule_name", "lhie1")
o:depends("rule_name", "ConnersHua")
o.rmempty = true
for groupname in string.gmatch(groupnames, "([^'##\n']+)##") do
  if groupname ~= nil and groupname ~= "" then
    o:value(groupname)
  end
end
o:value("DIRECT")
o:value("REJECT")

o = s:option(ListValue, "AsianTV", translate("AsianTV"))
o:depends("rule_name", "lhie1")
o:depends("rule_name", "ConnersHua")
o.rmempty = true
for groupname in string.gmatch(groupnames, "([^'##\n']+)##") do
  if groupname ~= nil and groupname ~= "" then
    o:value(groupname)
  end
end
o:value("DIRECT")
o:value("REJECT")

o = s:option(ListValue, "Proxy", translate("Proxy"))
o:depends("rule_name", "lhie1")
o:depends("rule_name", "ConnersHua")
o:depends("rule_name", "ConnersHua_return")
o.rmempty = true
for groupname in string.gmatch(groupnames, "([^'##\n']+)##") do
  if groupname ~= nil and groupname ~= "" then
    o:value(groupname)
  end
end
o:value("DIRECT")
o:value("REJECT")

o = s:option(ListValue, "Youtube", translate("Youtube"))
o:depends("rule_name", "lhie1")
o.rmempty = true
for groupname in string.gmatch(groupnames, "([^'##\n']+)##") do
  if groupname ~= nil and groupname ~= "" then
    o:value(groupname)
  end
end
o:value("DIRECT")
o:value("REJECT")

o = s:option(ListValue, "Apple", translate("Apple"))
o:depends("rule_name", "lhie1")
o.rmempty = true
for groupname in string.gmatch(groupnames, "([^'##\n']+)##") do
  if groupname ~= nil and groupname ~= "" then
    o:value(groupname)
  end
end
o:value("DIRECT")
o:value("REJECT")

o = s:option(ListValue, "Microsoft", translate("Microsoft"))
o:depends("rule_name", "lhie1")
o.rmempty = true
for groupname in string.gmatch(groupnames, "([^'##\n']+)##") do
  if groupname ~= nil and groupname ~= "" then
    o:value(groupname)
  end
end
o:value("DIRECT")
o:value("REJECT")

o = s:option(ListValue, "Netflix", translate("Netflix"))
o:depends("rule_name", "lhie1")
o.rmempty = true
for groupname in string.gmatch(groupnames, "([^'##\n']+)##") do
  if groupname ~= nil and groupname ~= "" then
    o:value(groupname)
  end
end
o:value("DIRECT")
o:value("REJECT")

o = s:option(ListValue, "Disney", translate("Disney"))
o:depends("rule_name", "lhie1")
o.rmempty = true
for groupname in string.gmatch(groupnames, "([^'##\n']+)##") do
  if groupname ~= nil and groupname ~= "" then
    o:value(groupname)
  end
end
o:value("DIRECT")
o:value("REJECT")

o = s:option(ListValue, "Spotify", translate("Spotify"))
o:depends("rule_name", "lhie1")
o.rmempty = true
for groupname in string.gmatch(groupnames, "([^'##\n']+)##") do
  if groupname ~= nil and groupname ~= "" then
    o:value(groupname)
  end
end
o:value("DIRECT")
o:value("REJECT")

o = s:option(ListValue, "Steam", translate("Steam"))
o:depends("rule_name", "lhie1")
o.rmempty = true
for groupname in string.gmatch(groupnames, "([^'##\n']+)##") do
  if groupname ~= nil and groupname ~= "" then
    o:value(groupname)
  end
end
o:value("DIRECT")
o:value("REJECT")

o = s:option(ListValue, "Speedtest", translate("Speedtest"))
o:depends("rule_name", "lhie1")
o.rmempty = true
for groupname in string.gmatch(groupnames, "([^'##\n']+)##") do
  if groupname ~= nil and groupname ~= "" then
    o:value(groupname)
  end
end
o:value("DIRECT")
o:value("REJECT")

o = s:option(ListValue, "Telegram", translate("Telegram"))
o:depends("rule_name", "lhie1")
o.rmempty = true
for groupname in string.gmatch(groupnames, "([^'##\n']+)##") do
  if groupname ~= nil and groupname ~= "" then
    o:value(groupname)
  end
end
o:value("DIRECT")
o:value("REJECT")

o = s:option(ListValue, "PayPal", translate("PayPal"))
o:depends("rule_name", "lhie1")
o.rmempty = true
for groupname in string.gmatch(groupnames, "([^'##\n']+)##") do
  if groupname ~= nil and groupname ~= "" then
    o:value(groupname)
  end
end
o:value("DIRECT")
o:value("REJECT")

o = s:option(ListValue, "AdBlock", translate("AdBlock"))
o:depends("rule_name", "lhie1")
o.rmempty = true
for groupname in string.gmatch(groupnames, "([^'##\n']+)##") do
  if groupname ~= nil and groupname ~= "" then
    o:value(groupname)
  end
end
o:value("DIRECT")
o:value("REJECT")

o = s:option(ListValue, "Domestic", translate("Domestic"))
o:depends("rule_name", "lhie1")
o:depends("rule_name", "ConnersHua")
o.rmempty = true
for groupname in string.gmatch(groupnames, "([^'##\n']+)##") do
  if groupname ~= nil and groupname ~= "" then
    o:value(groupname)
  end
end
o:value("DIRECT")
o:value("REJECT")

o = s:option(ListValue, "Others", translate("Others"))
o:depends("rule_name", "lhie1")
o:depends("rule_name", "ConnersHua")
o:depends("rule_name", "ConnersHua_return")
o.rmempty = true
o.description = translate("Choose Proxy Groups, Base On Your Config File").." ( "..font_green..bold_on..filename..bold_off..font_off.." )"
for groupname in string.gmatch(groupnames, "([^'##\n']+)##") do
  if groupname ~= nil and groupname ~= "" then
    o:value(groupname)
  end
end
o:value("DIRECT")
o:value("REJECT")

end

local t = {
    {Commit, Back}
}
a = m:section(Table, t)

o = a:option(Button,"Commit")
o.inputtitle = translate("Commit Configurations")
o.inputstyle = "apply"
o.write = function()
   m.uci:commit(openclash)
   --luci.http.redirect(m.redirect)
end

o = a:option(Button,"Back")
o.inputtitle = translate("Back Configurations")
o.inputstyle = "reset"
o.write = function()
   m.uci:revert(openclash)
   luci.http.redirect(m.redirect)
end

return m