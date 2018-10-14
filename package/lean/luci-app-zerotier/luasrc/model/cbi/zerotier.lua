local e=require"nixio.fs"
local e=luci.http
local o=require"luci.model.network".init()
local a,t,e,b

a=Map("zerotier",translate("ZeroTier"),translate("Zerotier is an open source, cross-platform and easy to use virtual LAN"))
a:section(SimpleSection).template  = "zerotier/zerotier_status"

t=a:section(NamedSection,"sample_config","zerotier")
t.anonymous=true
t.addremove=false

t:tab("basic",  translate("Base Setting"))

e=t:taboption("basic", Flag,"enabled",translate("Enable"))
e.default=0
e.rmempty=false

e=t:taboption("basic", DynamicList,"join",translate('ZeroTier Network ID'))
e.password=true
e.rmempty=false

e=t:taboption("basic", Flag,"nat",translate("Auto NAT Clients"))
e.default=0
e.rmempty=false
e.description = translate("Allow zerotier clients access your LAN network")

e=t:taboption("basic", DummyValue,"opennewwindow" , 
	translate("<input type=\"button\" class=\"cbi-button cbi-button-apply\" value=\"Zerotier.com\" onclick=\"window.open('https://my.zerotier.com/network')\" />"))
e.description = translate("Create or manage your zerotier network, and auth clients who could access")

local dog = "/tmp/zero.info"
t:tab("watchdog",  translate("Interface Info"))
log = t:taboption("watchdog", TextValue, "sylogtext")
log.template = "cbi/tvalue"
log.rows = 8
log.wrap = "off"
log.readonly="readonly"

function log.cfgvalue(self, section)
  luci.sys.exec("ifconfig $(ifconfig | grep zt | awk '{print $1}') > /tmp/zero.info")
	return nixio.fs.readfile(dog) or ""
end

function log.write(self, section, value)
	value = value:gsub("\r\n?", "\n")
	nixio.fs.writefile(dog, value)
end

return a
