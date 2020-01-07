local i="v2ray_server"
local n=require"luci.dispatcher"
local a,t,e
local o={
"none",
"srtp",
"utp",
"wechat-video",
"dtls",
"wireguard",
}
a=Map(i,"V2ray "..translate("Server Config"))
a.redirect=n.build_url("admin","vpn","v2ray_server")

t=a:section(NamedSection,arg[1],"user","")
t.addremove=false
t.dynamic=false

e=t:option(Flag,"enable",translate("Enable"))
e.default="1"
e.rmempty=false

e=t:option(Value,"remarks",translate("Remarks"))
e.default=translate("Remarks")

e.rmempty=false
e=t:option(Value,"port",translate("Port"))
e.datatype="port"
e.rmempty=false
e.default=10086

e=t:option(ListValue,"protocol",translate("Protocol"))
e:value("vmess",translate("Vmess"))
e:value("socks",translate("Socks"))
e:value("http",translate("Http"))

e=t:option(Value,"VMess_id",translate("ID"))
e.default=luci.sys.exec("cat /proc/sys/kernel/random/uuid")
e.rmempty=true
e:depends("protocol","vmess")

e=t:option(Value,"VMess_alterId",translate("Alter ID"))
e.default=16
e.rmempty=true
e:depends("protocol","vmess")

e=t:option(Value,"Socks_user",translate("User name"))
e.default="lean"
e.rmempty=true
e:depends("protocol","socks")

e=t:option(Value,"Socks_pass",translate("Password"))
e.default="password"
e.rmempty=true
e.password=true
e:depends("protocol","socks")

e=t:option(Value,"Http_user",translate("User name"))
e.default="lean"
e.rmempty=true
e:depends("protocol","http")

e=t:option(Value,"Http_pass",translate("Password"))
e.default="password"
e.rmempty=true
e.password=true
e:depends("protocol","http")

e=t:option(Value,"VMess_level",translate("User Level"))
e.default=1

e=t:option(ListValue,"transport",translate("Transport"))
e.default=tcp
e:value("tcp","TCP")
e:value("mkcp","mKCP")
e:value("quic","QUIC")
e:depends("protocol","vmess")

e=t:option(ListValue,"tcp_guise",translate("Camouflage Type"))
e:depends("transport","tcp")
e:value("none","none")
e:value("http","http")
e.default=none

e=t:option(DynamicList,"tcp_guise_http_host",translate("HTTP Host"))
e:depends("tcp_guise","http")
e=t:option(DynamicList,"tcp_guise_http_path",translate("HTTP Path"))
e:depends("tcp_guise","http")
e=t:option(ListValue,"mkcp_guise",translate("Camouflage Type"))
for a,t in ipairs(o)do e:value(t)end
e:depends("transport","mkcp")
e=t:option(Value,"mkcp_mtu",translate("KCP MTU"))
e:depends("transport","mkcp")
e=t:option(Value,"mkcp_tti",translate("KCP TTI"))
e:depends("transport","mkcp")
e=t:option(Value,"mkcp_uplinkCapacity",translate("KCP uplinkCapacity"))
e:depends("transport","mkcp")
e=t:option(Value,"mkcp_downlinkCapacity",translate("KCP downlinkCapacity"))
e:depends("transport","mkcp")
e=t:option(Flag,"mkcp_congestion",translate("KCP Congestion"))
e:depends("transport","mkcp")
e=t:option(Value,"mkcp_readBufferSize",translate("KCP readBufferSize"))
e:depends("transport","mkcp")
e=t:option(Value,"mkcp_writeBufferSize",translate("KCP writeBufferSize"))
e:depends("transport","mkcp")
e=t:option(ListValue,"quic_security",translate("Encrypt Method"))
e:value("none")
e:value("aes-128-gcm")
e:value("chacha20-poly1305")
e:depends("transport","quic")
e=t:option(Value,"quic_key",translate("Encrypt Method")..translate("Key"))
e:depends("transport","quic")
e=t:option(ListValue,"quic_guise",translate("Camouflage Type"))
for a,t in ipairs(o)do e:value(t)end
e:depends("transport","quic")

return a
