local a,t,e
local m, s
local o=require"nixio.fs"
local n={
"none",
"aes-128-ctr",
"aes-192-ctr",
"aes-256-ctr",
"aes-128-cfb",
"aes-192-cfb",
"aes-256-cfb",
"rc4",
"rc4-md5",
"rc4-md5-6",
}
local s={
"origin",
"verify_deflate",
"auth_sha1_v4",
"auth_aes128_md5",
"auth_aes128_sha1",
"auth_chain_a",
"auth_chain_b",
"auth_chain_c",
"auth_chain_d",
}
local i={
"plain",
"http_simple",
"http_post",
"random_head",
"tls1.2_ticket_auth",
"tls1.2_ticket_fastauth",
}
local o={
"false",
"true",
}

local running=(luci.sys.call("ps | grep server.py |grep -v grep >/dev/null") == 0)
if running then	
	a= Map("ssr_server", translate("ShadowSocksR Server Config"), translate("<b><font color=green>SSR Server is running.</font></b>"))
else
	a = Map("ssr_server", translate("ShadowSocksR Server Config"), translate("<b><font color=red>SSR Server is not running.</font></b>"))
end

t=a:section(TypedSection,"server",translate(""))
t.anonymous=true
t.addremove=false

e=t:option(Flag,"enable",translate("Enable"))
e.rmempty=false

e=t:option(Value,"server_port",translate("Server Port"))
e.datatype="port"
e.rmempty=false
e.default=139

e=t:option(Value,"password",translate("Password"))
e.password=true
e.rmempty=false

e=t:option(ListValue,"encrypt_method",translate("Encrypt Method"))
for a,t in ipairs(n)do e:value(t)end
e.rmempty=false

e=t:option(ListValue,"protocol",translate("Protocol"))
for a,t in ipairs(s)do e:value(t)end
e.rmempty=false

e=t:option(Value,"protocol_param",translate("Protocol_param"))
e.rmempty=true

e=t:option(ListValue,"obfs",translate("Obfs"))
for a,t in ipairs(i)do e:value(t)end
e.rmempty=false

e=t:option(Value,"obfs_param",translate("Obfs_param"))
e.rmempty=true



--e=t:option(Value,"redirect",translate("redirect"))
--e.rmempty=true

--e=t:option(Value,"timeout",translate("Connection Timeout"))
--e.datatype="uinteger"
--e.default=300
--e.rmempty=false

-- ---------------------------------------------------
--local apply = luci.http.formvalue("cbi.apply")
--if apply then
	--os.execute("/etc/init.d/ssr_server restart >/dev/null 2>&1 &")
--end

return a
