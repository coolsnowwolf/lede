-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Copyright 2013 Manuel Munz <freifunk at somakoma dot de>
-- Copyright 2014-2015 Christian Schoenebeck <christian dot schoenebeck at gmail dot com>
-- Licensed to the public under the Apache License 2.0.

local UCI  = (require "luci.model.uci").cursor()
local NX   = require "nixio"
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local UTIL = require "luci.util"
local DISP = require "luci.dispatcher"
local DTYP = require "luci.cbi.datatypes"

local apply = luci.http.formvalue("cbi.apply")               
if apply then                                                
        os.execute("/etc/init.d/ngrokc reload &")       -- reload configuration
end 

-- takeover arguments -- #######################################################
local section	= arg[1]

m = Map("ngrokc")
m.redirect = DISP.build_url("admin", "services", "ngrokc")

tunnels = m:section( NamedSection, section, "tunnel", "<h3>" .. translate("Details") .. " : " .. section .. "</h3>")
tunnels.instance = section	-- arg [1]

enabled=tunnels:option(Flag, "enabled", translate("Enable"))
enabled.anonymous = true
enabled.addremove = false

server=tunnels:option(ListValue, "server", translate("Server"))
--server:value("tunnel_mobi", "tunnel.mobi:44433")
--server:value("tunnel_org_cn", "tunnel.org.cn:4443")
UCI.foreach("ngrokc", "servers", function(s) server:value(s['.name'], s['.name'] .. " ( " .. s.host .. ":" .. s.port .. " ) ") end)


ptype=tunnels:option(ListValue, "type", translate("Type"))
ptype:value("tcp", translate("TCP"))
ptype:value("http", translate("HTTP"))
ptype:value("https", translate("HTTPS"))

lhost=tunnels:option(Value, "lhost", translate("Local Address"))
lhost.rmempty = true
lhost.placeholder="127.0.0.1"
lhost.datatype = "ip4addr"

lport=tunnels:option(Value, "lport", translate("Local Port"))
lport.datatype = "port"
lport.rmempty = false

custom_domain=tunnels:option(Flag, "custom_domain", translate("Use Custom Domain"))
custom_domain.default = "0"
custom_domain.disabled = "0"
custom_domain.enabled = "1"
custom_domain.rmempty = false
custom_domain:depends("type", "http")
custom_domain:depends("type", "https")

dname=tunnels:option(Value, "dname", translate("Custom Domain") .. "/" .. translate("SubDomain"), translate("Please set your domain's CNAME or A record to the tunnel server."))
dname.datatype = "hostname"
--dname.rmempty = false
dname.rmempty = true
dname:depends("type", "http")
dname:depends("type", "https")

rport=tunnels:option(Value, "rport", translate("Remote Port"))
rport.datatype = "port"
--rport.rmempty = false
rport.rmempty = true
rport:depends("type", "tcp")

custom_html=tunnels:option(DummyValue, "none")
custom_html.template = "ngrokc/ngrokc_script"

return m
