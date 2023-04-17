
local f, s, o
local clash = "clash"
local uci = luci.model.uci.cursor()
local fs = require "nixio.fs"
local sys = require "luci.sys"
local sid = arg[1]
local http = luci.http
local fss = require "luci.clash"
local NXFS = require "nixio.fs"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"


m = Map("clash", translate("Rule Providers List"))
m.reset = false
m.submit = false

local t = {
    {Apply}
}

a = m:section(Table, t)

o = a:option(Button, "Apply")
o.inputtitle = translate("Back to Overview")
o.inputstyle = "reset"
o.write = function()
  HTTP.redirect(DISP.build_url("admin", "services", "clash",  "config", "create"))
end


sys.call("awk -F ',' '{print $4}' /usr/share/clash/create/rule_provider.list > /tmp/rule_providers 2>/dev/null")
file = io.open("/tmp/rule_providers", "r");
local e={},a,o,t
a=nixio.fs.access("/tmp/rule_providers")
if a then
for o in file:lines() do
table.insert(e,o)
end
for t,o in ipairs(e) do
e[t]={}
e[t].num=string.format(t)


e[t].name=string.sub(luci.sys.exec(string.format("grep -F ',%s' /usr/share/clash/create/rule_provider.list |awk -F ',' '{print $1}' 2>/dev/null",o)),1,-2)

e[t].behaviour=string.sub(luci.sys.exec(string.format("grep -F ',%s' /usr/share/clash/create/rule_provider.list |awk -F ',' '{print $2}' 2>/dev/null",o)),1,-2)

e[t].filename=string.sub(luci.sys.exec(string.format("grep -F '%s,' /usr/share/clash/create/rule_provider.list |awk -F ',' '{print $4}' 2>/dev/null",o)),1,-2)

if e[t].filename == "" then
e[t].filename=o
end

RULE_FILE="/etc/clash/ruleprovider/".. e[t].filename
if fss.mtime(RULE_FILE) then
e[t].mtime=os.date("%Y-%m-%d %H:%M:%S",fss.mtime(RULE_FILE))
else
e[t].mtime="/"
end
if fss.isfile(RULE_FILE) then
   e[t].exist=translate("Exist")
else
   e[t].exist=translate("Not Exist")
end
e[t].remove=0
end
end
file:close()


f=Form("filelist")
tb=f:section(Table,e)
nu=tb:option(DummyValue,"num",translate("Order Number"))
st=tb:option(DummyValue,"exist",translate("State"))
nm=tb:option(DummyValue,"name",translate("Rule Name"))
tp=tb:option(DummyValue,"behaviour",translate("Type"))
fm=tb:option(DummyValue,"filename",translate("File Name"))
mt=tb:option(DummyValue,"mtime",translate("Update Time"))

btnis=tb:option(DummyValue,"filename",translate("Download Rule"))
btnis.template="clash/rulep"

btnrm=tb:option(Button,"remove",translate("Remove"))
btnrm.render=function(e,t,a)
e.inputstyle="reset"
Button.render(e,t,a)
end
btnrm.write=function(a,t)
local a=fs.unlink("/etc/clash/ruleprovider/"..e[t].filename)
HTTP.redirect(luci.dispatcher.build_url("admin", "services", "clash", "rulemanager"))
end

return m, f
