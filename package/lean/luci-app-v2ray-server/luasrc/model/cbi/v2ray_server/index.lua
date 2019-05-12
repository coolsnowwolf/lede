local o = require "luci.dispatcher"
local fs = require "nixio.fs"
local sys = require "luci.sys"
local cursor = luci.model.uci.cursor()
local appname = "v2ray_server"
local a,t,e

a=Map(appname, translate("V2ray Server"))

t=a:section(TypedSection,"global",translate("Global Settings"))
t.anonymous=true
t.addremove=false

e=t:option(Flag,"enable",translate("Enable"))
e.rmempty=false

t:append(Template("v2ray_server/v2ray"))

t=a:section(TypedSection,"user",translate("Users Manager"))
t.anonymous=true
t.addremove=true
t.template="cbi/tblsection"
t.extedit=o.build_url("admin","vpn",appname,"config","%s")
function t.create(e,t)
	local e=TypedSection.create(e,t)
	luci.http.redirect(o.build_url("admin","vpn",appname,"config",e))
end

function t.remove(t,a)
	t.map.proceed=true
	t.map:del(a)
	luci.http.redirect(o.build_url("admin","vpn",appname))
end

e=t:option(Flag, "enable", translate("Enable"))
e.width="5%"
e.rmempty = false

e=t:option(DummyValue,"status",translate("Status"))
e.template="v2ray_server/users_status"
e.value=translate("Collecting data...")

e=t:option(DummyValue,"remarks",translate("Remarks"))
e.width="15%"

e=t:option(DummyValue,"port",translate("Port"))
e.width="10%"

e=t:option(DummyValue,"protocol",translate("Protocol"))
e.width="15%"

e=t:option(DummyValue,"VMess_id",translate("ID"))
e.width="35%"

a:append(Template("v2ray_server/users_list_status"))

return a
