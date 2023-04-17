
local k, m, s, o, f
local clash = "clash"
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local fs = require "luci.clash"
local uci = require "luci.model.uci".cursor()

font_red = [[<font color="red">]]
font_blue = [[<font color="blue">]]
font_off = [[</font>]]
bold_on  = [[<strong>]]
bold_off = [[</strong>]]


m = Map(clash)
m.pageaction = false
m.description=''..font_red..bold_on..translate("Usage Steps:")..bold_off..font_off..' '
.."<br />"
..font_blue..bold_on..translate("1.Load Policy Group, make sure you have selected a config on config page")..bold_off..font_off..' '
.."<br />"
..font_blue..bold_on..translate("2.Download the game rules you want to use from the game rules list on this page")..bold_off..font_off..' '
.."<br />"
..font_blue..bold_on..translate("3. Make sure your game and servers support UDP")..bold_off..font_off..' '
.."<br />"
..font_blue..bold_on..translate("4.Click on ADD, Set the corresponding policy group of the downloaded game rules and save, recommended: FallBack")..bold_off..font_off..' '
.."<br />"
..font_blue..bold_on..translate("5.Select ClashTun Core type and start client")..bold_off..font_off..' '


SYS.call("awk -F ',' '{print $1}' /usr/share/clash/rules/rules.list > /tmp/rules_name 2>/dev/null")
file = io.open("/tmp/rules_name", "r");


r = m:section(TypedSection, "conf_groups", translate("Policy Groups"))
r.anonymous = true
r.addremove = false
r.sortable = false
r.template = "cbi/tblsection"
--r.extedit = luci.dispatcher.build_url("admin/services/clash/g-groups/%s")
function r.create(...)
	local sid = TypedSection.create(...)
	if sid then
		luci.http.redirect(r.extedit % sid)
		return
	end
end

o = r:option(DummyValue, "type", translate("Group Type"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end


o = r:option(DummyValue, "name", translate("Group Name"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end



-- [[ Edit Game Rule ]] --
r = m:section(TypedSection, "game", translate("Game Rules & Groups"))
r.anonymous = true
r.addremove = true
r.sortable = false
r.template = "cbi/tblsection"
r.extedit = luci.dispatcher.build_url("admin/services/clash/g-rules/%s")
function r.create(...)
	local sid = TypedSection.create(...)
	if sid then
		luci.http.redirect(r.extedit % sid)
		return
	end
end

o = r:option(DummyValue, "tag", translate("Game Rule Tag"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end


o = r:option(DummyValue, "group", translate("Policy Groups"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end


---- Rules List
local e={},a,o,t
a=nixio.fs.access("/tmp/rules_name")
if a then
for o in file:lines() do
table.insert(e,o)
end
for t,o in ipairs(e) do
e[t]={}
e[t].num=string.format(t)
e[t].name=o
e[t].filename=string.sub(luci.sys.exec(string.format("grep -F '%s,' /usr/share/clash/rules/rules.list |awk -F ',' '{print $3}' 2>/dev/null",e[t].name)),1,-2)
if e[t].filename == "" then
e[t].filename=string.sub(luci.sys.exec(string.format("grep -F '%s,' /usr/share/clash/rules/rules.list |awk -F ',' '{print $2}' 2>/dev/null",e[t].name)),1,-2)
end
RULE_FILE="/usr/share/clash/rules/g_rules/".. e[t].filename
if fs.mtime(RULE_FILE) then
e[t].mtime=os.date("%Y-%m-%d %H:%M:%S",fs.mtime(RULE_FILE))
else
e[t].mtime="/"
end
if fs.isfile(RULE_FILE) then
   e[t].exist=translate("Exist")
else
   e[t].exist=translate("Not Exist")
end
e[t].remove=0
end
end
file:close()


f=Form("filelist",  translate("Game Rules List"))
tb=f:section(Table,e)
nu=tb:option(DummyValue,"num",translate("Order Number"))
st=tb:option(DummyValue,"exist",translate("State"))
--nm=tb:option(DummyValue,"name",translate("Rule Name")) 
fm=tb:option(DummyValue,"filename",translate("File Name"))
mt=tb:option(DummyValue,"mtime",translate("Update Time"))

btnis=tb:option(DummyValue,"filename",translate("Download Rule"))
btnis.template="clash/game_rule"

btnrm=tb:option(Button,"remove",translate("Remove"))
btnrm.render=function(e,t,a)
e.inputstyle="reset"
Button.render(e,t,a)
end
btnrm.write=function(a,t)
local a=fs.unlink("/usr/share/clash/rules/g_rules/"..e[t].filename)
HTTP.redirect(luci.dispatcher.build_url("admin", "services", "clash", "settings", "grules"))
end

local t = {
    {Load,Apply}
}

k = Form("apply")
k.reset = false
k.submit = false
s = k:section(Table, t)


o = s:option(Button, "Load") 
o.inputtitle = translate("Load Groups")
o.inputstyle = "apply"
o.write = function()
  m.uci:commit("clash")
  luci.sys.call("bash /usr/share/clash/load_groups.sh >/dev/null 2>&1 &")
  luci.sys.call("sleep 3")
  HTTP.redirect(luci.dispatcher.build_url("admin", "services", "clash", "settings", "grules"))   
end

o = s:option(Button, "Apply")
o.inputtitle = translate("Save & Apply")
o.inputstyle = "apply"
o.write = function()
  m.uci:commit("clash")
  if luci.sys.call("pidof clash >/dev/null") == 0 then
	SYS.call("/etc/init.d/clash restart >/dev/null 2>&1 &")
        luci.http.redirect(luci.dispatcher.build_url("admin", "services", "clash"))
  else
  	HTTP.redirect(luci.dispatcher.build_url("admin", "services", "clash", "settings", "grules"))
  end
end

return m, k, f

