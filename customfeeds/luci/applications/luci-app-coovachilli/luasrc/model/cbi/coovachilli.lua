-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

m = Map("coovachilli")

-- general
s = m:section(TypedSection, "general")
s.anonymous = true

s:option( Flag, "debug" )
s:option( Value, "interval" )
s:option( Value, "pidfile" ).optional = true
s:option( Value, "statedir" ).optional = true
s:option( Value, "cmdsock" ).optional = true
s:option( Value, "logfacility" ).optional = true


return m
