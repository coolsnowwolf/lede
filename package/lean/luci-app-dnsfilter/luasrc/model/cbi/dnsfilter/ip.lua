local fs=require "nixio.fs"
local conffile="/etc/dnsfilter/ip.list"

f=SimpleForm("custom")
t=f:field(TextValue,"conf")
t.rmempty=true
t.rows=13
t.description=translate("Will Always block these IP")
function t.cfgvalue()
	return fs.readfile(conffile) or ""
end

function f.handle(self,state,data)
	if state == FORM_VALID then
		if data.conf then
			fs.writefile(conffile,data.conf:gsub("\r\n","\n"))
		else
			luci.sys.call("> /etc/dnsfilter/ip.list")
		end
		luci.sys.exec("ipset -F blockip 2>/dev/null && for i in $(cat /etc/dnsfilter/ip.list);do ipset add blockip $i 2>/dev/null;done")
	end
	return true
end

return f
