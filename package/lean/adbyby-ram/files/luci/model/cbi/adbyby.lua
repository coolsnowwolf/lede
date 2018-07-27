
local m, s, o,oo
local Run_flag=0
local lazy_time=translate("Not exist")
local video_time=translate("Not exist")
local sys = require "luci.sys"
icount=tonumber(sys.exec("ps -w | grep adbyby |grep -v grep| wc -l"))
if tonumber(icount)>0 then
icount=tonumber(sys.exec("netstat -nautp | grep adbyby |grep -v grep| wc -l"))
if tonumber(icount)>0 then
Run_flag=1
else
Run_flag=2
end
end

if nixio.fs.access("/tmp/adbyby/bin/data/lazy.txt") then
time_cmd="awk  'NR==1{print $3\" \"$4}' /tmp/adbyby/bin/data/lazy.txt"
lazy_time=sys.exec(time_cmd)
end

if nixio.fs.access("/tmp/adbyby/bin/data/video.txt") then
time_cmd="awk  'NR==1{print $3\" \"$4}' /tmp/adbyby/bin/data/video.txt"
video_time=sys.exec(time_cmd)
end
m = Map("adbyby", translate("AD Block"))

    
s = m:section(NamedSection, "adbyby","adbyby", "")
s.addremove = false

oo = s:option(ListValue, "enable",translate("Enable Adbyby"))
oo:value("0", translate("Disable"))
oo:value("1", translate("Enable"))
oo.widget = "radio"
oo.advance_second_title = true
oo.orientation = "horizontal"

o=s:option(DummyValue, "runstatus",translate("Running Status"))
o.rawhtml  = true
if Run_flag==1 then
o.value="<font color='green'>" .. translate("Running") .. "</font>"
elseif  Run_flag==0 then
o.value="<font color='red'>" .. translate("Not Running") .. "</font>"
else
o.value="<font color='blue'>" .. translate("Wait Connect..") .. "</font>"
end

o=s:option(DummyValue, "version1",translate("Lazy rules time")) 
o.rawhtml  = true
o.value="<font color='blue'>" .. lazy_time .. "</font>"

o=s:option(DummyValue, "version2",translate("Video rules time"))
o.rawhtml  = true
o.value="<font color='blue'>" .. video_time .. "</font>"

local apply = luci.http.formvalue("cbi.apply")
if apply then
local m_enable=luci.http.formvalue("cbid.adbyby.adbyby.enable")
 if m_enable=="0" then
 luci.sys.call("/etc/init.d/adbyby_ram disable")
 luci.sys.call("/etc/init.d/adbyby_ram stop")
 else
 luci.sys.call("/etc/init.d/adbyby_ram enable")
 luci.sys.call("/etc/init.d/adbyby_ram start")
 end


--luci.http.redirect(luci.dispatcher.build_url("admin/services/adbyby")) 
end

return m
