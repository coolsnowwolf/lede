
local form, m
local openclash = "openclash"
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local fs = require "luci.openclash"
local uci = require "luci.model.uci".cursor()

m = SimpleForm("openclash", translate("Other Rule Providers List"))
m.description=translate("Rule Project:").." ConnersHua ( https://github.com/DivineEngine/Profiles )<br/>"..
translate("Rule Project:").." lhie1 ( https://github.com/lhie1/Rules )"
m.reset = false
m.submit = false

local t = {
    {Apply}
}

a = m:section(Table, t)

o = a:option(Button, "Refresh", " ")
o.inputtitle = translate("Refresh Page")
o.inputstyle = "apply"
o.write = function()
  HTTP.redirect(DISP.build_url("admin", "services", "openclash", "rule-providers-manage"))
end

o = a:option(Button, "Apply", " ")
o.inputtitle = translate("Back Settings")
o.inputstyle = "reset"
o.write = function()
  HTTP.redirect(DISP.build_url("admin", "services", "openclash", "rule-providers-settings"))
end

if not NXFS.access("/tmp/rule_providers_name") then
   SYS.call("awk -v d=',' -F ',' '{print $4d$5}' /usr/share/openclash/res/rule_providers.list > /tmp/rule_providers_name 2>/dev/null")
end
file = io.open("/tmp/rule_providers_name", "r");

local function i(e)
local t=0
local a={' KB',' MB',' GB',' TB'}
repeat
e=e/1024
t=t+1
until(e<=1024)
return string.format("%.1f",e)..a[t]
end

---- Rules List
local e={},o,t
if NXFS.access("/tmp/rule_providers_name") then
for o in file:lines() do
table.insert(e,o)
end
for t,o in ipairs(e) do
e[t]={}
e[t].num=string.format(t)
e[t].name=string.sub(luci.sys.exec(string.format("grep -F '%s' /usr/share/openclash/res/rule_providers.list |awk -F ',' '{print $1}' 2>/dev/null",o)),1,-2)
e[t].lfilename=string.sub(luci.sys.exec(string.format("grep -F '%s' /usr/share/openclash/res/rule_providers.list |awk -F ',' '{print $6}' 2>/dev/null",o)),1,-2)
if e[t].lfilename == "" then
e[t].lfilename=string.sub(luci.sys.exec(string.format("grep -F '%s' /usr/share/openclash/res/rule_providers.list |awk -F ',' '{print $5}' 2>/dev/null",o)),1,-2)
end
e[t].filename=o
e[t].author=string.sub(luci.sys.exec(string.format("grep -F '%s' /usr/share/openclash/res/rule_providers.list |awk -F ',' '{print $2}' 2>/dev/null",o)),1,-2)
e[t].rule_type=string.sub(luci.sys.exec(string.format("grep -F '%s' /usr/share/openclash/res/rule_providers.list |awk -F ',' '{print $3}' 2>/dev/null",o)),1,-2)
RULE_FILE="/etc/openclash/rule_provider/".. e[t].lfilename
if fs.mtime(RULE_FILE) then
e[t].size=i(fs.stat(RULE_FILE).size)
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
nu=tb:option(DummyValue,"num",translate("Order Number"))
st=tb:option(DummyValue,"exist",translate("State"))
st.template="openclash/cfg_check"
tp=tb:option(DummyValue,"rule_type",translate("Rule Type"))
nm=tb:option(DummyValue,"name",translate("Rule Name"))
au=tb:option(DummyValue,"author",translate("Rule Author"))
fm=tb:option(DummyValue,"lfilename",translate("File Name"))
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
fs.unlink("/etc/openclash/rule_provider/"..e[t].lfilename)
HTTP.redirect(DISP.build_url("admin", "services", "openclash", "rule-providers-manage"))
end

return m, form
