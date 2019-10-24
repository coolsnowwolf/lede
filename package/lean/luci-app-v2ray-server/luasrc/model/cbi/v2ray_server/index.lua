local i=require"luci.dispatcher"
local e=require"nixio.fs"
local e=require"luci.sys"
local e=luci.model.uci.cursor()
local o="v2ray_server"
local a,e,t

a=Map(o,translate("V2ray Server"))
e=a:section(TypedSection,"global",translate("Global Setting"))
e.anonymous=true
e.addremove=false
t=e:option(Flag,"enable",translate("Enable"))
t.rmempty=false

e=a:section(TypedSection,"user",translate("Server Setting"))
e.anonymous=true
e.addremove=true
e.template="cbi/tblsection"
e.extedit=i.build_url("admin","vpn",o,"config","%s")

function e.create(t,e)
local e=TypedSection.create(t,e)
luci.http.redirect(i.build_url("admin","vpn",o,"config",e))
end

function e.remove(e,a)
e.map.proceed=true
e.map:del(a)
luci.http.redirect(i.build_url("admin","vpn",o))
end

t=e:option(Flag,"enable",translate("Enable"))
t.width="5%"
t.rmempty=false
t=e:option(DummyValue,"status",translate("Status"))
t.template="v2ray_server/users_status"
t.value=translate("Collecting data...")
t=e:option(DummyValue,"remarks",translate("Remarks"))
t.width="15%"
t=e:option(DummyValue,"port",translate("Port"))
t=e:option(DummyValue,"protocol",translate("Protocol"))

a:append(Template("v2ray_server/users_list_status"))

return a
