local api = require "luci.passwall.api"
local appname = api.appname
local sys = api.sys
local has_chnlist = api.fs.access("/usr/share/passwall/rules/chnlist")

m = Map(appname)
api.set_apply_on_parse(m)

s = m:section(TypedSection, "global", translate("ACLs"), "<font color='red'>" .. translate("ACLs is a tools which used to designate specific IP proxy mode.") .. "</font>")
s.anonymous = true

o = s:option(Flag, "acl_enable", translate("Main switch"))
o.rmempty = false
o.default = false

local global_proxy_mode = (m:get("@global[0]", "tcp_proxy_mode") or "") .. (m:get("@global[0]", "udp_proxy_mode") or "")

-- [[ ACLs Settings ]]--
s = m:section(TypedSection, "acl_rule")
s.template = "cbi/tblsection"
s.sortable = true
s.anonymous = true
s.addremove = true
s.extedit = api.url("acl_config", "%s")
function s.create(e, t)
	t = TypedSection.create(e, t)
	luci.http.redirect(e.extedit:format(t))
end
function s.remove(e, t)
	sys.call("rm -rf /tmp/etc/passwall_tmp/dns_" .. t .. "*")
	TypedSection.remove(e, t)
end

---- Enable
o = s:option(Flag, "enabled", translate("Enable"))
o.default = 1
o.rmempty = false

---- Remarks
o = s:option(Value, "remarks", translate("Remarks"))
o.rmempty = true

local mac_t = {}
sys.net.mac_hints(function(e, t)
	mac_t[e] = {
		ip = t,
		mac = e
	}
end)

o = s:option(DummyValue, "sources", translate("Source"))
o.rawhtml = true
o.cfgvalue = function(t, n)
	local e = ''
	local v = Value.cfgvalue(t, n) or ''
	string.gsub(v, '[^' .. " " .. ']+', function(w)
		local a = w
		if mac_t[w] then
			a = a .. ' (' .. mac_t[w].ip .. ')'
		end
		if #e > 0 then
			e = e .. "<br />"
		end
		e = e .. a
	end)
	return e
end

---- TCP Proxy Mode
tcp_proxy_mode = s:option(ListValue, "tcp_proxy_mode", "TCP " .. translate("Proxy Mode"))
tcp_proxy_mode.default = "default"
tcp_proxy_mode.rmempty = false
tcp_proxy_mode:value("default", translate("Default"))
tcp_proxy_mode:value("disable", translate("No Proxy"))
tcp_proxy_mode:value("global", translate("Global Proxy"))
if has_chnlist and global_proxy_mode:find("returnhome") then
	tcp_proxy_mode:value("returnhome", translate("China List"))
else
	tcp_proxy_mode:value("gfwlist", translate("GFW List"))
	tcp_proxy_mode:value("chnroute", translate("Not China List"))
end
tcp_proxy_mode:value("direct/proxy", translate("Only use direct/proxy list"))

---- UDP Proxy Mode
udp_proxy_mode = s:option(ListValue, "udp_proxy_mode", "UDP " .. translate("Proxy Mode"))
udp_proxy_mode.default = "default"
udp_proxy_mode.rmempty = false
udp_proxy_mode:value("default", translate("Default"))
udp_proxy_mode:value("disable", translate("No Proxy"))
udp_proxy_mode:value("global", translate("Global Proxy"))
if has_chnlist and global_proxy_mode:find("returnhome") then
	udp_proxy_mode:value("returnhome", translate("China List"))
else
	udp_proxy_mode:value("gfwlist", translate("GFW List"))
	udp_proxy_mode:value("chnroute", translate("Not China List"))
end
udp_proxy_mode:value("direct/proxy", translate("Only use direct/proxy list"))

--[[
---- TCP No Redir Ports
o = s:option(Value, "tcp_no_redir_ports", translate("TCP No Redir Ports"))
o.default = "default"
o:value("disable", translate("No patterns are used"))
o:value("default", translate("Default"))
o:value("1:65535", translate("All"))

---- UDP No Redir Ports
o = s:option(Value, "udp_no_redir_ports", translate("UDP No Redir Ports"))
o.default = "default"
o:value("disable", translate("No patterns are used"))
o:value("default", translate("Default"))
o:value("1:65535", translate("All"))

---- TCP Redir Ports
o = s:option(Value, "tcp_redir_ports", translate("TCP Redir Ports"))
o.default = "default"
o:value("default", translate("Default"))
o:value("1:65535", translate("All"))
o:value("80,443", "80,443")
o:value("80:65535", "80 " .. translate("or more"))
o:value("1:443", "443 " .. translate("or less"))

---- UDP Redir Ports
o = s:option(Value, "udp_redir_ports", translate("UDP Redir Ports"))
o.default = "default"
o:value("default", translate("Default"))
o:value("1:65535", translate("All"))
o:value("53", "53")
]]--

return m
