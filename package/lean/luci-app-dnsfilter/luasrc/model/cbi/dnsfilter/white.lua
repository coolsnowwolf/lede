local fs=require "nixio.fs"
local conffile="/etc/dnsfilter/white.list"

f=SimpleForm("custom")
t=f:field(TextValue,"conf")
t.rmempty=true
t.rows=13
t.description=translate("Will Never filter these Domain")
function t.cfgvalue()
	return fs.readfile(conffile) or ""
end

function f.handle(self,state,data)
	if state == FORM_VALID then
		if data.conf then
			fs.writefile(conffile,data.conf:gsub("\r\n","\n"))
		else
			luci.sys.call("> /etc/dnsfilter/white.list")
		end
		luci.sys.exec("for i in $(cat /etc/dnsfilter/white.list);do sed -i -e \"/\\/$i\\//d\" -e \"/\\.$i\\//d\" /tmp/dnsfilter/rules.conf 2>/dev/null;\\\
		[ -s /etc/dnsfilter/rules/rules.conf ] && sed -i -e \"/\\/$i\\//d\" -e \"/\\.$i\\//d\" /etc/dnsfilter/rules/rules.conf;done;\\\
		[ -s /tmp/dnsfilter/rules.conf ] && rm -f /tmp/dnsmasq.dnsfilter/rules.conf && /etc/init.d/dnsfilter start")
	end
	return true
end

return f
