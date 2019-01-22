-- Copyright (C) 2017 yushi studio <ywb94@qq.com>
-- Copyright (C) 2018 openwrt-brook-tproxy
-- Licensed to the public under the GNU General Public License v3.

local m, s, o
local brook = "brook"
local uci = luci.model.uci.cursor()
local ipkg = require("luci.model.ipkg")
local sid = arg[1]

local server_table = {}

m = Map(brook, translate("Edit Brook Server"))
m.redirect = luci.dispatcher.build_url("admin/services/brook/client")
if m.uci:get(brook, sid) ~= "servers" then
	luci.http.redirect(m.redirect)
	return
end

-- [[ Servers Setting ]]--
s = m:section(NamedSection, sid, "servers")
s.anonymous = true
s.addremove   = false

o = s:option(Value, "alias", translate("Alias(optional)"))

o = s:option(Value, "server", translate("Server Address"))
o.datatype = "host"
o.rmempty = false

o = s:option(Value, "server_port", translate("Server Port"))
o.datatype = "port"
o.rmempty = false

o = s:option(Value, "local_port", translate("Local Port"))
o.datatype = "port"
o.default = 1080
o.rmempty = false

o = s:option(Value, "password", translate("Password"))
o.password = true
o.rmempty = false

return m