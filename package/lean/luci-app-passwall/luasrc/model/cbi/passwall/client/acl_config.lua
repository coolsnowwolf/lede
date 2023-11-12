local api = require "luci.passwall.api"
local appname = api.appname
local sys = api.sys
local has_singbox = api.finded_com("singbox")
local has_xray = api.finded_com("xray")
local has_chnlist = api.fs.access("/usr/share/passwall/rules/chnlist")

m = Map(appname)
api.set_apply_on_parse(m)

local nodes_table = {}
for k, e in ipairs(api.get_valid_nodes()) do
	nodes_table[#nodes_table + 1] = e
end

local global_proxy_mode = (m:get("@global[0]", "tcp_proxy_mode") or "") .. (m:get("@global[0]", "udp_proxy_mode") or "")

local dynamicList_write = function(self, section, value)
	local t = {}
	local t2 = {}
	if type(value) == "table" then
		local x
		for _, x in ipairs(value) do
			if x and #x > 0 then
				if not t2[x] then
					t2[x] = x
					t[#t+1] = x
				end
			end
		end
	else
		t = { value }
	end
	t = table.concat(t, " ")
	return DynamicList.write(self, section, t)
end

-- [[ ACLs Settings ]]--
s = m:section(NamedSection, arg[1], translate("ACLs"), translate("ACLs"))
s.addremove = false
s.dynamic = false

---- Enable
o = s:option(Flag, "enabled", translate("Enable"))
o.default = 1
o.rmempty = false

---- Remarks
o = s:option(Value, "remarks", translate("Remarks"))
o.default = arg[1]
o.rmempty = true

local mac_t = {}
sys.net.mac_hints(function(e, t)
	mac_t[#mac_t + 1] = {
		ip = t,
		mac = e
	}
end)
table.sort(mac_t, function(a,b)
	if #a.ip < #b.ip then
		return true
	elseif #a.ip == #b.ip then
		if a.ip < b.ip then
			return true
		else
			return #a.ip < #b.ip
		end
	end
	return false
end)

---- Source
sources = s:option(DynamicList, "sources", translate("Source"))
sources.description = "<ul><li>" .. translate("Example:")
.. "</li><li>" .. translate("MAC") .. ": 00:00:00:FF:FF:FF"
.. "</li><li>" .. translate("IP") .. ": 192.168.1.100"
.. "</li><li>" .. translate("IP CIDR") .. ": 192.168.1.0/24"
.. "</li><li>" .. translate("IP range") .. ": 192.168.1.100-192.168.1.200"
.. "</li><li>" .. translate("IPSet") .. ": ipset:lanlist"
.. "</li></ul>"
sources.cast = "string"
for _, key in pairs(mac_t) do
	sources:value(key.mac, "%s (%s)" % {key.mac, key.ip})
end
sources.cfgvalue = function(self, section)
	local value
	if self.tag_error[section] then
		value = self:formvalue(section)
	else
		value = self.map:get(section, self.option)
		if type(value) == "string" then
			local value2 = {}
			string.gsub(value, '[^' .. " " .. ']+', function(w) table.insert(value2, w) end)
			value = value2
		end
	end
	return value
end
sources.validate = function(self, value, t)
	local err = {}
	for _, v in ipairs(value) do
		local flag = false
		if v:find("ipset:") and v:find("ipset:") == 1 then
			local ipset = v:gsub("ipset:", "")
			if ipset and ipset ~= "" then
				flag = true
			end
		end

		if flag == false and datatypes.macaddr(v) then
			flag = true
		end

		if flag == false and datatypes.ip4addr(v) then
			flag = true
		end

		if flag == false and api.iprange(v) then
			flag = true
		end

		if flag == false then
			err[#err + 1] = v
		end
	end

	if #err > 0 then
		self:add_error(t, "invalid", translate("Not true format, please re-enter!"))
		for _, v in ipairs(err) do
			self:add_error(t, "invalid", v)
		end
	end

	return value
end
sources.write = dynamicList_write

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

---- TCP Proxy Drop Ports
o = s:option(Value, "tcp_proxy_drop_ports", translate("TCP Proxy Drop Ports"))
o.default = "default"
o:value("disable", translate("No patterns are used"))
o:value("default", translate("Default"))

---- UDP Proxy Drop Ports
o = s:option(Value, "udp_proxy_drop_ports", translate("UDP Proxy Drop Ports"))
o.default = "default"
o:value("disable", translate("No patterns are used"))
o:value("default", translate("Default"))
o:value("80,443", translate("QUIC"))

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

tcp_node = s:option(ListValue, "tcp_node", "<a style='color: red'>" .. translate("TCP Node") .. "</a>")
tcp_node.default = "default"
tcp_node:value("default", translate("Default"))

udp_node = s:option(ListValue, "udp_node", "<a style='color: red'>" .. translate("UDP Node") .. "</a>")
udp_node.default = "default"
udp_node:value("default", translate("Default"))
udp_node:value("tcp", translate("Same as the tcp node"))

for k, v in pairs(nodes_table) do
	tcp_node:value(v.id, v["remark"])
	udp_node:value(v.id, v["remark"])
end

o = s:option(Flag, "filter_proxy_ipv6", translate("Filter Proxy Host IPv6"), translate("Experimental feature."))
o.default = "0"
o:depends({ tcp_node = "default",  ['!reverse'] = true })

---- DNS Forward Mode
o = s:option(ListValue, "dns_mode", translate("Filter Mode"))
o:depends({ tcp_node = "default",  ['!reverse'] = true })
if api.is_finded("dns2socks") then
	o:value("dns2socks", "dns2socks")
end
if has_singbox then
	o:value("sing-box", "Sing-Box")
end
if has_xray then
	o:value("xray", "Xray")
end

o = s:option(ListValue, "xray_dns_mode", " ")
o:value("tcp", "TCP")
o:value("tcp+doh", "TCP + DoH (" .. translate("A/AAAA type") .. ")")
o:depends("dns_mode", "xray")
o.cfgvalue = function(self, section)
	return m:get(section, "v2ray_dns_mode")
end
o.write = function(self, section, value)
	if s.fields["dns_mode"]:formvalue(section) == "xray" then
		return m:set(section, "v2ray_dns_mode", value)
	end
end

o = s:option(ListValue, "singbox_dns_mode", " ")
o:value("tcp", "TCP")
o:value("doh", "DoH")
o:depends("dns_mode", "sing-box")
o.cfgvalue = function(self, section)
	return m:get(section, "v2ray_dns_mode")
end
o.write = function(self, section, value)
	if s.fields["dns_mode"]:formvalue(section) == "sing-box" then
		return m:set(section, "v2ray_dns_mode", value)
	end
end

---- DNS Forward
o = s:option(Value, "remote_dns", translate("Remote DNS"))
o.default = "1.1.1.1"
o:value("1.1.1.1", "1.1.1.1 (CloudFlare)")
o:value("1.1.1.2", "1.1.1.2 (CloudFlare-Security)")
o:value("8.8.4.4", "8.8.4.4 (Google)")
o:value("8.8.8.8", "8.8.8.8 (Google)")
o:value("9.9.9.9", "9.9.9.9 (Quad9-Recommended)")
o:value("208.67.220.220", "208.67.220.220 (OpenDNS)")
o:value("208.67.222.222", "208.67.222.222 (OpenDNS)")
o:depends({dns_mode = "dns2socks"})
o:depends({xray_dns_mode = "tcp"})
o:depends({xray_dns_mode = "tcp+doh"})
o:depends({singbox_dns_mode = "tcp"})

if has_singbox or has_xray then
	o = s:option(Value, "remote_dns_doh", translate("Remote DNS DoH"))
	o:value("https://1.1.1.1/dns-query", "CloudFlare")
	o:value("https://1.1.1.2/dns-query", "CloudFlare-Security")
	o:value("https://8.8.4.4/dns-query", "Google 8844")
	o:value("https://8.8.8.8/dns-query", "Google 8888")
	o:value("https://9.9.9.9/dns-query", "Quad9-Recommended")
	o:value("https://208.67.222.222/dns-query", "OpenDNS")
	o:value("https://dns.adguard.com/dns-query,176.103.130.130", "AdGuard")
	o:value("https://doh.libredns.gr/dns-query,116.202.176.26", "LibreDNS")
	o:value("https://doh.libredns.gr/ads,116.202.176.26", "LibreDNS (No Ads)")
	o.default = "https://1.1.1.1/dns-query"
	o.validate = function(self, value, t)
		if value ~= "" then
			value = api.trim(value)
			local flag = 0
			local util = require "luci.util"
			local val = util.split(value, ",")
			local url = val[1]
			val[1] = nil
			for i = 1, #val do
				local v = val[i]
				if v then
					if not api.datatypes.ipmask4(v) then
						flag = 1
					end
				end
			end
			if flag == 0 then
				return value
			end
		end
		return nil, translate("DoH request address") .. " " .. translate("Format must be:") .. " URL,IP"
	end
	o:depends({xray_dns_mode = "tcp+doh"})
	o:depends({singbox_dns_mode = "doh"})

	if has_xray then
		o = s:option(Value, "dns_client_ip", translate("EDNS Client Subnet"))
		o.datatype = "ipaddr"
		o:depends({dns_mode = "xray"})
	end
end

if api.is_finded("chinadns-ng") then
	o = s:option(Flag, "chinadns_ng", translate("ChinaDNS-NG"), translate("The effect is better, but will increase the memory."))
	o.default = "0"
	o:depends({ tcp_proxy_mode = "gfwlist", dns_mode = "dns2socks" })
	o:depends({ tcp_proxy_mode = "gfwlist", dns_mode = "xray" })
	o:depends({ tcp_proxy_mode = "gfwlist", dns_mode = "sing-box" })
	o:depends({ tcp_proxy_mode = "chnroute", dns_mode = "dns2socks" })
	o:depends({ tcp_proxy_mode = "chnroute", dns_mode = "xray" })
	o:depends({ tcp_proxy_mode = "chnroute", dns_mode = "sing-box" })
	chinadns_ng_default_tag = s:option(ListValue, "chinadns_ng_default_tag", translate("ChinaDNS-NG Domain Default Tag"))
	chinadns_ng_default_tag.default = "smart"
	chinadns_ng_default_tag:value("smart", translate("Smart DNS"))
	chinadns_ng_default_tag:value("gfw", translate("Remote DNS"))
	chinadns_ng_default_tag:value("chn", translate("Direct DNS"))
	chinadns_ng_default_tag.description = "<ul>"
			.. "<li>" .. translate("Forward to both remote and direct DNS, if the direct DNS resolution result is a mainland China ip, then use the direct result, otherwise use the remote result") .. "</li>"
			.. "<li>" .. translate("Remote DNS can avoid more DNS leaks, but some domestic domain names maybe to proxy!") .. "</li>"
			.. "<li>" .. translate("Direct DNS Internet experience may be better, but DNS will be leaked!") .. "</li>"
			.. "</ul>"
	chinadns_ng_default_tag:depends("chinadns_ng", true)
end

if has_chnlist then
	when_chnroute_default_dns = s:option(ListValue, "when_chnroute_default_dns", translate("When using the chnroute list the default DNS"))
	when_chnroute_default_dns.default = "direct"
	when_chnroute_default_dns:value("remote", translate("Remote DNS"))
	when_chnroute_default_dns:value("direct", translate("Direct DNS"))
	when_chnroute_default_dns.description = "<ul>"
	.. "<li>" .. translate("Remote DNS can avoid more DNS leaks, but some domestic domain names maybe to proxy!") .. "</li>"
	.. "<li>" .. translate("Direct DNS Internet experience may be better, but DNS will be leaked!") .. "</li>"
	.. "</ul>"
	local _depends = {
		{ dns_mode = "dns2socks" },
		{ dns_mode = "xray" },
		{ dns_mode = "sing-box" },
	}
	for i, d in ipairs(_depends) do
		d["tcp_proxy_mode"] = "chnroute"
		if api.is_finded("chinadns-ng") then
			d["chinadns_ng"] = false
		end
		when_chnroute_default_dns:depends(d)
	end
end

return m
