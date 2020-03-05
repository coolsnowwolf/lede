-- Copyright 2020 Weizheng Li <lwz322@qq.com>
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

o = s:option(Value, "bind_udp_port", translate("UDP bind port"), 
    translatef("Optional: udp port to help make udp hole to penetrate nat"))
o.datatype = "port"

o = s:option(Value, "kcp_bind_port", translate("KCP bind port"), 
    translatef("Optional: udp port used for kcp protocol, it can be same with 'bind port'; if not set, kcp is disabled in frps"))
o.datatype = "port"

o = s:option(Value, "vhost_http_port", translate("vhost http port"), 
    translatef("Optional: if you want to support virtual host, you must set the http port for listening"))
o.datatype = "port"

o = s:option(Value, "vhost_https_port", translate("vhost https port"), 
    translatef("Optional:  Note: http port and https port can be same with bind_port"))
o.datatype = "port"

return m