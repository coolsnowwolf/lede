
local form, m
local openclash = "openclash"
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local fs = require "luci.openclash"
local uci = require "luci.model.uci".cursor()

m = SimpleForm("openclash", translate("Game Rules List"))
m.description=translate("Rule Project:").." SSTap-Rule ( https://github.com/FQrabbit/SSTap-Rule )"
m.reset = false
m.submit = false

local t = {
    {Refresh, Apply}
}

a = m:section(Table, t)

o = a:option(Button, "Refresh", " ")
o.inputtitle = translate("Refresh Page")
o.inputstyle = "apply"
o.write = function()
   SYS.call("rm -rf /tmp/rules_name 2>/dev/null")
   HTTP.redirect(DISP.build_url("admin", "services", "openclash", "game-rules-manage"))
end

o = a:option(Button, "Apply", " ")
o.inputtitle = translate("Back Settings")
o.inputstyle = "reset"
o.write = function()
  HTTP.redirect(DISP.build_url("admin", "services", "openclash", "rule-providers-settings"))
end

if not NXFS.access("/tmp/rules_name") then
   SYS.call("awk -F ',' '{print $1}' /usr/share/openclash/res/game_rules.list > /tmp/rules_name 2>/dev/null")
end
file = io.open("/tmp/rules_name", "r");

---- Rules List
local e={},o,t
if NXFS.access("/tmp/rules_name") then
for o in file:lines() do
table.insert(e,o)
end
for t,o in ipairs(e) do
e[t]={}
e[t].num=string.format(t)
e[t].name=o
e[t].filename=string.sub(luci.sys.exec(string.format("grep -F '%s,' /usr/share/openclash/res/game_rules.list |awk -F ',' '{print $3}' 2>/dev/null",e[t].name)),1,-2)
if e[t].filename == "" then
e[t].filename=string.sub(luci.sys.exec(string.format("grep -F '%s,' /usr/share/openclash/res/game_rules.list |awk -F ',' '{print $2}' 2>/dev/null",e[t].name)),1,-2)
end
RULE_FILE="/etc/openclash/game_rules/".. e[t].filename
if fs.mtime(RULE_FILE) then
e[t].size=fs.filesize(fs.stat(RULE_FILE).size)
e[t].mtime=os.date("%Y-%m-%d %H:%M:%S",fs.mtime(RULE_FILE))
else
e[t].size="/"
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

form=SimpleForm("filelist")
form.reset=false
form.submit=false
tb=form:section(Table,e)
nu=tb:option(DummyValue,"num",translate("Serial Number"))
st=tb:option(DummyValue,"exist",translate("State"))
st.template="openclash/cfg_check"
nm=tb:option(DummyValue,"name",translate("Rule Name"))
fm=tb:option(DummyValue,"filename",translate("File Name"))
sz=tb:option(DummyValue,"size",translate("Size"))
mt=tb:option(DummyValue,"mtime",translate("Update Time"))

btnis=tb:option(DummyValue,"filename",translate("Download Rule"))
btnis.template="openclash/download_rule"

btnrm=tb:option(Button,"remove",translate("Remove"))
btnrm.render=function(e,t,a)
e.inputstyle="reset"
Button.render(e,t,a)
end
btnrm.write=function(a,t)
fs.unlink("/etc/openclash/game_rules/"..e[t].filename)
HTTP.redirect(DISP.build_url("admin", "services", "openclash", "game-rules-manage"))
end

return m, form
