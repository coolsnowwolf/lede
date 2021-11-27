local o = require"luci.dispatcher"
local e = require("luci.model.ipkg")
local s = require"nixio.fs"
local e = luci.model.uci.cursor()
local i = "frp"
local a,t,e
local n = {}

a = Map("frp")
a.title = translate("Frp Setting")
a.description = translate("Frp is a fast reverse proxy to help you expose a local server behind a NAT or firewall to the internet.")

a:section(SimpleSection).template="frp/frp_status"

t = a:section(NamedSection, "common","frp")
t.anonymous = true
t.addremove = false

t:tab("base", translate("Basic Settings"))
t:tab("other", translate("Other Settings"))
t:tab("log", translate("Client Log"))

e = t:taboption("base", Flag, "enabled", translate("Enabled"))
e.rmempty = false

e = t:taboption("base", Value, "server_addr", translate("Server"))
e.optional = false
e.rmempty = false

e = t:taboption("base", Value, "server_port", translate("Port"))
e.datatype = "port"
e.optional = false
e.rmempty = false

e = t:taboption("base", Value, "token", translate("Token"))
e.description = translate("Time duration between server of frpc and frps mustn't exceed 15 minutes.")
e.optional = false
e.password = true
e.rmempty = false

e = t:taboption("base", Value, "user", translate("User"))
e.description = translate("Commonly used to distinguish you with other clients.")
e.optional = true
e.default = ""
e.rmempty = false

e = t:taboption("base", Value, "vhost_http_port", translate("Vhost HTTP Port"))
e.datatype = "port"
e.rmempty = false

e = t:taboption("base", Value, "vhost_https_port", translate("Vhost HTTPS Port"))
e.datatype = "port"
e.rmempty = false

e = t:taboption("base", Value, "time", translate("Service registration interval"))
e.description = translate("0 means disable this feature, unit: min")
e.datatype = "range(0,59)"
e.default = 30
e.rmempty = false

e = t:taboption("other", Flag, "login_fail_exit", translate("Exit program when first login failed"))
e.description = translate("decide if exit program when first login failed, otherwise continuous relogin to frps.")
e.default = "1"
e.rmempty = false

e = t:taboption("other", Flag, "tcp_mux", translate("TCP Stream Multiplexing"))
e.description = translate("Default is Ture. This feature in frps.ini and frpc.ini must be same.")
e.default = "1"
e.rmempty = false

e = t:taboption("other", Flag, "tls_enable", translate("Use TLS Connection"))
e.description = translate("if tls_enable is true, frpc will connect frps by tls.")
e.default = "0"
e.rmempty = false

e = t:taboption("other", ListValue, "protocol", translate("Protocol Type"))
e.description = translate("Frp support kcp protocol since v0.12.0")
e.default = "tcp"
e:value("tcp", translate("TCP Protocol"))
e:value("kcp", translate("KCP Protocol"))

e = t:taboption("other", Flag, "enable_http_proxy", translate("Connect frps by HTTP PROXY"))
e.description = translate("frpc can connect frps using HTTP PROXY")
e.default = "0"
e.rmempty = false
e:depends("protocol","tcp")

e = t:taboption("other", Value, "http_proxy", translate("HTTP PROXY"))
e.datatype = "uinteger"
e.placeholder = "http://user:pwd@192.168.1.128:8080"
e:depends("enable_http_proxy",1)
e.optional = false

e = t:taboption("other", Flag, "enable_cpool", translate("Enable Connection Pool"))
e.description = translate("This feature is fit for a large number of short connections.")
e.rmempty = false

e = t:taboption("other", Value, "pool_count", translate("Connection Pool"))
e.description = translate("Connections will be established in advance.")
e.datatype = "uinteger"
e.default = "1"
e:depends("enable_cpool",1)
e.optional=false

e = t:taboption("other", ListValue, "log_level", translate("Log Level"))
e.default = "warn"
e:value("trace", translate("Trace"))
e:value("debug", translate("Debug"))
e:value("info", translate("Info"))
e:value("warn", translate("Warning"))
e:value("error", translate("Error"))

e = t:taboption("other", Value, "log_max_days", translate("Log Keepd Max Days"))
e.datatype = "uinteger"
e.default = "3"
e.rmempty = false
e.optional = false

e = t:taboption("other", Flag, "admin_enable", translate("Enable Web API"))
e.description = translate("set admin address for control frpc's action by http api such as reload.")
e.default = "0"
e.rmempty=false

e = t:taboption("other", Value, "admin_port", translate("Admin Web Port"))
e.datatype = "port"
e.default = 7400
e.rmempty = false
e:depends("admin_enable",1)

e = t:taboption("other", Value, "admin_user", translate("Admin Web UserName"))
e.optional = false
e.default = "admin"
e.rmempty=false
e:depends("admin_enable",1)

e = t:taboption("other", Value, "admin_pwd", translate("Admin Web PassWord"))
e.optional = false
e.default = "admin"
e.password = true
e.rmempty = false
e:depends("admin_enable",1)

e = t:taboption("log", TextValue,"log")
e.rows = 26
e.wrap = "off"
e.readonly = true
e.cfgvalue = function(t,t)
return s.readfile("/var/etc/frp/frpc.log")or""
end
e.write = function(e,e,e)
end

t = a:section(TypedSection, "proxy", translate("Services List"))
t.anonymous = true
t.addremove = true
t.template = "cbi/tblsection"
t.extedit = o.build_url("admin","services","frp","config","%s")

function t.create(e,t)
new = TypedSection.create(e,t)
luci.http.redirect(e.extedit:format(new))
end

function t.remove(e,t)
e.map.proceed = true
e.map:del(t)
luci.http.redirect(o.build_url("admin","services","frp"))
end

local o = ""
e = t:option(DummyValue, "remark", translate("Service Remark Name"))
e.width="10%"

e = t:option(DummyValue, "type", translate("Frp Protocol Type"))
e.width="10%"

e = t:option(DummyValue, "custom_domains", translate("Domain/Subdomain"))
e.width="20%"

e.cfgvalue = function(t,n)
local t = a.uci:get(i,n,"domain_type")or""
local m = a.uci:get(i,n,"type")or""
if t=="custom_domains" then
local b = a.uci:get(i,n,"custom_domains")or"" return b end
if t=="subdomain" then
local b = a.uci:get(i,n,"subdomain")or"" return b end
if t=="both_dtype" then
local b = a.uci:get(i,n,"custom_domains")or""
local c = a.uci:get(i,n,"subdomain")or""
b="%s/%s"%{b,c} return b end
if m=="tcp" or m=="udp" then
local b=a.uci:get(i,"common","server_addr")or"" return b end
end

e = t:option(DummyValue,"remote_port",translate("Remote Port"))
e.width = "10%"
e.cfgvalue = function(t,b)
local t = a.uci:get(i,b,"type")or""
if t==""or b==""then return""end
if t=="http" then
local b = a.uci:get(i,"common","vhost_http_port")or"" return b end
if t=="https" then
local b = a.uci:get(i,"common","vhost_https_port")or"" return b end
if t=="tcp" or t=="udp" then
local b = a.uci:get(i,b,"remote_port")or"" return b end
end

e = t:option(DummyValue, "local_ip", translate("Local Host Address"))
e.width = "15%"

e = t:option(DummyValue, "local_port", translate("Local Host Port"))
e.width = "10%"

e = t:option(DummyValue, "use_encryption", translate("Use Encryption"))
e.width = "15%"

e.cfgvalue = function(t,n)
local t=a.uci:get(i,n,"use_encryption")or""
local b
if t==""or b==""then return""end
if t=="1" then b="ON"
else b="OFF" end
return b
end

e = t:option(DummyValue, "use_compression", translate("Use Compression"))
e.width = "15%"
e.cfgvalue = function(t,n)
local t = a.uci:get(i,n,"use_compression")or""
local b
if t==""or b==""then return""end
if t=="1" then b="ON"
else b="OFF" end
return b
end

e = t:option(Flag, "enable", translate("Enable State"))
e.width = "10%"
e.rmempty = false

return a
