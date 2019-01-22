-- Copyright (C) 2017 yushi studio <ywb94@qq.com>
-- Copyright (C) 2018 openwrt-brook-tproxy
-- Licensed to the public under the GNU General Public License v3.

local m, s, sec, o
local brook = "brook"
local uci = luci.model.uci.cursor()
local ipkg = require("luci.model.ipkg")

m = Map(brook, translate("Brook Tproxy"))

local server_table = {}

uci:foreach(brook, "servers", function(s)
	if s.alias then
		server_table[s[".name"]] = s.alias
	elseif s.server and s.server_port then
		server_table[s[".name"]] = "%s:%s" %{s.server, s.server_port}
	end
end)

-- [[ Servers Setting ]]--
sec = m:section(TypedSection, "servers", translate("Servers Setting"))
sec.anonymous = true
sec.addremove = true
sec.sortable = true
sec.template = "cbi/tblsection"
sec.extedit = luci.dispatcher.build_url("admin/services/brook/client/%s")
function sec.create(...)
	local sid = TypedSection.create(...)
	if sid then
		luci.http.redirect(sec.extedit % sid)
		return
	end
end

o = sec:option(DummyValue, "alias", translate("Alias"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end

o = sec:option(DummyValue, "server", translate("Server Address"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or "?"
end

o = sec:option(DummyValue, "server_port", translate("Server Port"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or "?"
end

-- [[ Global Setting ]]--
s = m:section(TypedSection, "global", translate("Global Setting"))
s.anonymous = true

o = s:option(ListValue, "global_server", translate("Global Server"))
o:value("nil", translate("Disable"))
for k, v in pairs(server_table) do o:value(k, v) end
o.default = "nil"
o.rmempty = false

o = s:option(ListValue, "run_mode", translate("Running Mode"))
o:value("router", translate("IP Route Mode"))
o:value("gfw", translate("GFW List Mode"))

o = s:option(Value, "tunnel_forward", translate("DNS Server IP and Port"))
o.default = "8.8.8.8:53"
o.rmempty = false

-- [[ Proxy Control ]]--
s = m:section(TypedSection, "proxy_control", translate("Proxy Control"))
s.anonymous = true

-- Part of WAN
s:tab("wan_pc", translate("Interfaces - WAN"))

o = s:taboption("wan_pc", Value, "wan_bp_list", translate("Bypassed IP List"))
o:value("/dev/null", translate("NULL - As Global Proxy"))

o.default = "/dev/null"
o.rmempty = false

o = s:taboption("wan_pc", DynamicList, "wan_bp_ips", translate("Bypassed IP"))
o.datatype = "ip4addr"

o = s:taboption("wan_pc", DynamicList, "wan_fw_ips", translate("Forwarded IP"))
o.datatype = "ip4addr"

-- Part of LAN
s:tab("lan_pc", translate("Interfaces - LAN"))

o = s:taboption("lan_pc", ListValue, "lan_pc_mode", translate("LAN Host Proxy Control"))
o:value("0", translate("Disable"))
o:value("w", translate("Allow listed only"))
o:value("b", translate("Allow all except listed"))
o.rmempty = false

o = s:taboption("lan_pc", DynamicList, "lan_pc_ips", translate("LAN Host List"))
o.datatype = "ipaddr"
luci.ip.neighbors({ family = 4 }, function(entry)
       if entry.reachable then
               o:value(entry.dest:string())
       end
end)

return m