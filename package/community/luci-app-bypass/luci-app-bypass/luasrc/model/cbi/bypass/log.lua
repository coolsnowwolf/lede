local fs=require "nixio.fs"

luci.sys.exec("sed -e '1!G;h;$!d' /var/log/bypass.log >/var/log/bypass_r.log")
f=SimpleForm("logview")
f.reset=false
f.submit=false
t=f:field(TextValue,"conf")
t.rmempty=true
t.rows=20
function t.cfgvalue()
	return fs.readfile("/var/log/bypass_r.log") or ""
end
t.readonly="readonly"

return f
