
local m, s, o
local openclash = "openclash"
local uci = luci.model.uci.cursor()
local fs = require "luci.openclash"
local sys = require "luci.sys"
local sid = arg[1]

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

m = Map(openclash, translate("Edit Group"))
m.pageaction = false
m.redirect = luci.dispatcher.build_url("admin/services/openclash/servers")
if m.uci:get(openclash, sid) ~= "groups" then
	luci.http.redirect(m.redirect)
	return
end

-- [[ Groups Setting ]]--
s = m:section(NamedSection, sid, "groups")
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

o = s:option(ListValue, "type", translate("Group Type"))
o.rmempty = true
o.description = translate("Choose The Operation Mode")
o:value("select", translate("Manual-Select"))
o:value("url-test", translate("URL-Test"))
o:value("fallback", translate("Fallback"))
o:value("load-balance", translate("Load-Balance"))
o:value("relay", translate("Relay-Traffic"))

o = s:option(ListValue, "strategy", translate("Strategy Type"))
o.rmempty = true
o.description = translate("Choose The Load-Balance's Strategy Type")
o:value("consistent-hashing", translate("Consistent-hashing"))
o:value("round-robin", translate("Round-robin"))
o:depends("type", "load-balance")

o = s:option(Value, "name", translate("Group Name"))
o.rmempty = false
o.default = "Group -"..sid

o = s:option(ListValue, "disable_udp", translate("Disable UDP"))
o:value("false", translate("Disable"))
o:value("true", translate("Enable"))
o.default = "false"
o.rmempty = false

o = s:option(Value, "test_url", translate("Test URL"))
o:value("http://www.gstatic.com/generate_204")
o:value("https://cp.cloudflare.com/generate_204")
o.rmempty = false
o:depends("type", "url-test")
o:depends("type", "fallback")
o:depends("type", "load-balance")

o = s:option(Value, "test_interval", translate("Test Interval(s)"))
o.default = "300"
o.rmempty = false
o:depends("type", "url-test")
o:depends("type", "fallback")
o:depends("type", "load-balance")

o = s:option(Value, "tolerance", translate("Tolerance(ms)"))
o.default = "150"
o.rmempty = true
o:depends("type", "url-test")

o = s:option(DynamicList, "other_group", translate("Other Group"))
o.description = font_red..bold_on..translate("The Added Proxy Groups Must Exist Except 'DIRECT' & 'REJECT'")..bold_off..font_off
uci:foreach("openclash", "groups",
		function(s)
		  if s.name ~= "" and s.name ~= nil and s.name ~= m.uci:get(openclash, sid, "name") then
			   o:value(s.name)
			end
		end)
o:value("DIRECT")
o:value("REJECT")
o.rmempty = true

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

return m