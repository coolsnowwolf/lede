-- Copyright 2020 lwz322 <lwz322@qq.com>
-- Licensed to the public under the MIT License.

local dsp = require "luci.dispatcher"

local m, s, o

m = Map("frps", "%s - %s" % { translate("Frps"), translate("FRPS Server setting") })

s = m:section(NamedSection, "main", "frps")
s.anonymous = true
s.addremove = false

o = s:option(Value, "bind_port", translate("Bind port"))
o.datatype = "port"
o.rmempty = false

o = s:option(Value, "token", translate("Token"))
o.password = true

o = s:option(Flag, "tcp_mux", translate("TCP mux"))
o.enabled = "true"
o.disabled = "false"
o.defalut = o.enabled
o.rmempty = false

o = s:option(Flag, "tls_only", translate("Enforce frps only accept TLS connection"))
o.description = translatef("Requirements: frpc v0.25.0+, frps v0.32.0+")
o.enabled = "true"
o.disabled = "false"
o.default = o.disabled
o.rmempty = false

o = s:option(Value, "bind_udp_port", translate("UDP bind port"))
o.description = translatef("Optional: udp port to help make udp hole to penetrate nat")
o.datatype = "port"

o = s:option(Value, "kcp_bind_port", translate("KCP bind port"))
o.description = translatef("Optional: udp port used for kcp protocol, it can be same with 'bind port'; if not set, kcp is disabled in frps")
o.datatype = "port"

o = s:option(Value, "vhost_http_port", translate("vhost http port")) 
o.description = translatef("Optional: if you want to support virtual host, you must set the http port for listening")
o.datatype = "port"

o = s:option(Value, "vhost_https_port", translate("vhost https port"))
o.description = translatef("Optional:  Note: http port and https port can be same with bind_port")
o.datatype = "port"

o = s:option(DynamicList, "extra_setting", translate("Extra Settings"))
o.description = translatef("List of extra settings will be added to config file. Format: option=value, eg. <code>detailed_errors_to_client=false</code>.(NO SPACE!)")
o.placeholder = "option=value"

return m