local m, s, o
local clash = "clash"
local uci = luci.model.uci.cursor()
--local fs = require "nixio.fs"
local sys = require "luci.sys"
local sid = arg[1]
local fs = require "luci.clash"

m = Map(clash, translate("Edit Game Rule & Group"))
--m.pageaction = false
m.redirect = luci.dispatcher.build_url("admin/services/clash/settings/grules")
if m.uci:get(clash, sid) ~= "game" then
	luci.http.redirect(m.redirect)
	return
end

function IsRuleFile(e)
e=e or""
local e=string.lower(string.sub(e,-6,-1))
return e==".rules"
end

-- [[ Groups Setting ]]--
s = m:section(NamedSection, sid, "game")
s.anonymous = true
s.addremove   = false

o = s:option(Value, "tag")
o.title = translate("Rule Tag")
o.rmempty = false
o.description = translate("Give a special name for your rules")

---- Proxy Group
o = s:option(ListValue, "group", translate("Select Proxy Group"))
uci:foreach("clash", "conf_groups",
		function(s)
		  if s.name ~= "" and s.name ~= nil then
			   o:value(s.name)
			end
		end)
o:value("DIRECT")
o:value("REJECT")
o.rmempty = true
o.description = translate("Select a policy group to add rule")

---- rule name
o = s:option(DynamicList, "rule_name", translate("Game Rule's Name"))
local e,a={}
for t,f in ipairs(fs.glob("/usr/share/clash/rules/g_rules/*"))do
	a=fs.stat(f)
	if a then
    e[t]={}
    e[t].filename=fs.basename(f)
    if IsRuleFile(e[t].filename) then
       e[t].name=luci.sys.exec(string.format("grep -F '%s' /usr/share/clash/rules/rules.list | awk -F ',' '{print $2}'",e[t].filename))
       o:value(e[t].filename)
    end
  end
end
   
o.rmempty = true


return m
