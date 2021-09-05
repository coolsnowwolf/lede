local m,s,o
local bypass="bypass"
local uci=luci.model.uci.cursor()
local server_count=0

uci:foreach(bypass,"servers",function(s)
	server_count=server_count+1
end)

m=Map(bypass,translate("Servers subscription and manage"),translate("Support SS/SSR/XRAY/TROJAN/TROJAN-GO/NAIVEPROXY/SOCKS5/TUN etc."))
s=m:section(TypedSection,"server_subscribe")
s.anonymous=true

o=s:option(Flag,"auto_update",translate("Auto Update"))
o.rmempty=false
o.description=translate("Auto Update Server subscription,GFW list and CHN route")

o=s:option(ListValue,"auto_update_time",translate("Update time (every day)"))
for t=0,23 do
	o:value(t,t..":00")
end
o.default=2
o.rmempty=false

o=s:option(DynamicList,"subscribe_url",translate("Subscribe URL"))
o.rmempty=true

o=s:option(ListValue,"filter_mode",translate("Filter Words Mode"))
o:value("",translate("Discard Mode"))
o:value(1,translate("Keep Mode"))

o=s:option(Value,"filter_words",translate("Subscribe Filter Words"))
o.rmempty=true
o.description=translate("Filter Words splited by /")

o=s:option(Button,"update_Sub",translate("Update Subscribe List"))
o.inputstyle="reload"
o.description=translate("Update subscribe url list first")
o.write=function()
	uci:commit(bypass)
	luci.http.redirect(luci.dispatcher.build_url("admin","services",bypass,"servers"))
end

o=s:option(Flag,"switch",translate("Subscribe Default Auto-Switch"))
o.rmempty=false
o.description=translate("Subscribe new add server default Auto-Switch on")
o.default=0

o=s:option(Flag,"proxy",translate("Through proxy update"))
o.rmempty=false
o.description=translate("Through proxy update list,Not Recommended")

o=s:option(Button,"subscribe",translate("Update All Subscribe Severs"))
o.rawhtml=true
o.template="bypass/subscribe"

o=s:option(Button,"delete",translate("Delete All Subscribe Severs"))
o.inputstyle="reset"
o.description=string.format(translate("Server Count")..": %d",server_count)
o.write=function()
	uci:delete_all(bypass,"servers",function(s)
		if s.hashkey or s.isSubscribe then
			return true
		else
			return false
		end
	end)
	uci:commit(bypass)
	luci.http.redirect(luci.dispatcher.build_url("admin","services",bypass,"servers"))
end

s=m:section(TypedSection,"servers")
s.anonymous=true
s.addremove=true
s.template="cbi/tblsection"
s.sortable=true
s.extedit=luci.dispatcher.build_url("admin","services",bypass,"servers","%s")
function s.create(...)
	local sid=TypedSection.create(...)
	if sid then
		uci:set(bypass,sid,'switch_enable',1)
		luci.http.redirect(s.extedit%sid)
		return
	end
end

o=s:option(DummyValue,"type",translate("Type"))
function o.cfgvalue(...)
	return (Value.cfgvalue(...)=="vless") and "VLESS" or Value.cfgvalue(...)
end

o=s:option(DummyValue,"alias",translate("Alias"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end

o=s:option(DummyValue,"server_port",translate("Server Port"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or "N/A"
end

o=s:option(DummyValue,"server_port",translate("Socket Connected"))
o.template="bypass/socket"
o.width="10%"

o=s:option(DummyValue,"server",translate("TCPing Latency"))
o.template="bypass/ping"
o.width="10%"

o=s:option(Button,"apply_node",translate("Apply"))
o.inputstyle="apply"
o.write=function(self,section)
	uci:set(bypass,'@global[0]','global_server',section)
	uci:commit(bypass)
	luci.http.redirect(luci.dispatcher.build_url("admin","services",bypass,"base"))
end

o=s:option(Flag,"switch_enable",translate("Auto Switch"))
o.rmempty=false
function o.cfgvalue(...)
	return Value.cfgvalue(...) or 1
end

m:append(Template("bypass/server_list"))

return m
