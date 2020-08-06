local a=require"luci.sys"
local e=luci.model.uci.cursor()
local e=require"nixio.fs"
require("luci.sys")
local t,e,o

t=Map("aliddns",translate("AliDDNS"))

e=t:section(TypedSection,"base",translate("Base"))
e.anonymous=true

enable=e:option(Flag,"enable",translate("enable"))
enable.rmempty=false

enable=e:option(Flag,"clean",translate("Clean Before Update"))
enable.rmempty=false

token=e:option(Value,"app_key",translate("Access Key ID"))
email=e:option(Value,"app_secret",translate("Access Key Secret"))

iface=e:option(ListValue,"interface",translate("WAN-IP Source"),translate("Select the WAN-IP Source for AliDDNS, like wan/internet"))
iface:value("",translate("Select WAN-IP Source"))
iface:value("internet")
iface:value("wan")

iface.rmempty=false
main=e:option(Value,"main_domain",translate("Main Domain"),translate("For example: test.github.com -> github.com"))
main.rmempty=false
sub=e:option(Value,"sub_domain",translate("Sub Domain"),translate("For example: test.github.com -> test"))
sub.rmempty=false
time=e:option(Value,"time",translate("Inspection Time"),translate("Unit: Minute, Range: 1-59"))
time.rmempty=false

e=t:section(TypedSection,"base",translate("Update Log"))
e.anonymous=true
local a="/var/log/aliddns.log"
tvlog=e:option(TextValue,"sylogtext")
tvlog.rows=16
tvlog.readonly="readonly"
tvlog.wrap="off"

function tvlog.cfgvalue(e,e)
	sylogtext=""
	if a and nixio.fs.access(a) then
		sylogtext=luci.sys.exec("tail -n 100 %s"%a)
	end
	return sylogtext
end

tvlog.write=function(e,e,e)
end
local e=luci.http.formvalue("cbi.apply")
if e then
	io.popen("/etc/init.d/aliddns restart")
end
return t
