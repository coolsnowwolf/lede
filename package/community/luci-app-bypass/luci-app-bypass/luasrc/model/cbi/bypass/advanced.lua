local server_table={}
luci.model.uci.cursor():foreach("bypass","servers",function(s)
	if (s.type=="ss" and not nixio.fs.access("/usr/bin/ss-local")) or (s.type=="ssr" and not nixio.fs.access("/usr/bin/ssr-local")) or s.type=="socks5" or s.type=="tun" then
		return
	end
	if s.alias then
		server_table[s[".name"]]="[%s]:%s"%{string.upper(s.type),s.alias}
	elseif s.server and s.server_port then
		server_table[s[".name"]]="[%s]:%s:%s"%{string.upper(s.type),s.server,s.server_port}
	end
end)

local key_table={}
for key,_ in pairs(server_table) do
    table.insert(key_table,key)
end

table.sort(key_table)

m=Map("bypass")

s=m:section(TypedSection,"global",translate("Server failsafe auto swith settings"))
s.anonymous=true

o=s:option(Flag,"monitor_enable",translate("Enable Process Deamon"))
o.default=1

o=s:option(Flag,"enable_switch",translate("Enable Auto Switch"))
o.default=1

o=s:option(Value,"switch_time",translate("Switch check cycly(second)"))
o.datatype="uinteger"
o.default=300
o:depends("enable_switch",1)

o=s:option(Value,"switch_timeout",translate("Check timout(second)"))
o.datatype="uinteger"
o.default=5
o:depends("enable_switch",1)

o=s:option(Value,"switch_try_count",translate("Check Try Count"))
o.datatype="uinteger"
o.default=3
o:depends("enable_switch",1)

s=m:section(TypedSection,"socks5_proxy",translate("Global SOCKS5 Proxy Server"))
s.anonymous=true

o=s:option(ListValue,"server",translate("Server"))
o:value("",translate("Disable"))
o:value("same",translate("Same as Global Server"))
for _,key in pairs(key_table) do o:value(key,server_table[key]) end

o=s:option(Value,"local_port",translate("Local Port"))
o.datatype="port"
o.placeholder=1080

return m
