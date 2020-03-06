m=Map("oafclog")
m.title=translate("应用过滤日志及配置文件")
s=m:section(TypedSection,"oafclog")
s.addremove=false
s.anonymous=true
s:tab("log",translate("日志"))
s.anonymous=true
local a="/tmp/oafControl/oafControl.log"
alog=s:taboption("log",TextValue,"oafclog")
alog.description=translate("应用过滤日志")
alog.rows=18
alog.wrap="off"
function alog.cfgvalue(s,s)
sylogtext=""
if a and nixio.fs.access(a)then
oafclog=luci.sys.exec("tail -n 100 %s"%a)
end
return oafclog
end
-------------------config file-----------------------
s:tab("config", translate("配置文件"))
s.anonymous=true
local va="/etc/oafControl/vacation.bak"
vconfig=s:taboption("config",TextValue,"Config_File")
vconfig.description=translate("应用过滤日志")
vconfig.rows=18
vconfig.wrap="off"
function vconfig.cfgvalue(s,s)
sylogtext=""
if va and nixio.fs.access(va)then
Config_File=luci.sys.exec("tail -n 100 %s"%va)
end
return Config_File
end
-- function vconfig.write(t,t,e)
-- e=e:gsub("\r\n?","\n")
-- nixio.fs.writefile("/etc/vlmcsd.ini",e)
-- end

local oa="/etc/oafControl/overtime.bak"
oconfig=s:taboption("config",TextValue,"Config_File")
oconfig.description=translate("应用过滤日志")
oconfig.rows=18
oconfig.wrap="off"
function oconfig.cfgvalue(s,s)
sylogtext=""
if oa and nixio.fs.access(oa)then
Config_File=luci.sys.exec("tail -n 100 %s"%oa)
end
return Config_File
end
-- function oconfig.write(t,t,e)
-- e=e:gsub("\r\n?","\n")
-- nixio.fs.writefile("/etc/vlmcsd.ini",e)
-- end

return m