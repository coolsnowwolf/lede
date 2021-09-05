local api = require "luci.model.cbi.passwall.api.api"
local appname = api.appname
local sys = api.sys
local has_v2ray = api.is_finded("v2ray")
local has_xray = api.is_finded("xray")
local has_chnlist = api.fs.access("/usr/share/passwall/rules/chnlist")

m = Map(appname)

local nodes_table = {}
for k, e in ipairs(api.get_valid_nodes()) do
    nodes_table[#nodes_table + 1] = e
end

local global_proxy_mode = (m:get("@global[0]", "tcp_proxy_mode") or "") .. (m:get("@global[0]", "udp_proxy_mode") or "")

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

o = s:option(DynamicList, "ip_mac", translate("IP/MAC"))
o.datatype = "or(ip4addr,macaddr)"
o.cast = "string"
o.rmempty = false

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
for _, key in pairs(mac_t) do
    o:value(key.mac, "%s (%s)" % {key.mac, key.ip})
end
function o.write(self, section, value)
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

---- TCP Proxy Mode
tcp_proxy_mode = s:option(ListValue, "tcp_proxy_mode", translatef("%s Proxy Mode", "TCP"))
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
udp_proxy_mode = s:option(ListValue, "udp_proxy_mode", translatef("%s Proxy Mode", "UDP"))
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

---- DNS Forward Mode
o = s:option(ListValue, "dns_mode", translate("Filter Mode"))
o:depends({ tcp_node = "default",  ['!reverse'] = true })
if api.is_finded("dns2socks") then
    o:value("dns2socks", "dns2socks")
end
if has_v2ray then
    o:value("v2ray_doh", "V2ray DNS(DoH)")
    o:value("v2ray_tcp", "V2ray DNS(TCP)")
end
if has_xray then
    o:value("xray_doh", "Xray DNS(DoH)")
end

---- DNS Forward
o = s:option(Value, "dns_forward", translate("Remote DNS"))
o.default = "8.8.8.8"
o:value("8.8.8.8", "8.8.8.8 (Google DNS)")
o:value("8.8.4.4", "8.8.4.4 (Google DNS)")
o:value("208.67.222.222", "208.67.222.222 (Open DNS)")
o:value("208.67.220.220", "208.67.220.220 (Open DNS)")
o:depends("dns_mode", "dns2socks")

if has_v2ray or has_xray then
---- DoH
o = s:option(Value, "dns_doh", translate("DoH request address"))
o:value("https://dns.adguard.com/dns-query,176.103.130.130", "AdGuard")
o:value("https://cloudflare-dns.com/dns-query,1.1.1.1", "Cloudflare")
o:value("https://security.cloudflare-dns.com/dns-query,1.1.1.2", "Cloudflare-Security")
o:value("https://doh.opendns.com/dns-query,208.67.222.222", "OpenDNS")
o:value("https://dns.google/dns-query,8.8.8.8", "Google")
o:value("https://doh.libredns.gr/dns-query,116.202.176.26", "LibreDNS")
o:value("https://doh.libredns.gr/ads,116.202.176.26", "LibreDNS (No Ads)")
o:value("https://dns.quad9.net/dns-query,9.9.9.9", "Quad9-Recommended")
o.default = "https://dns.google/dns-query,8.8.8.8"
o.validate = function(self, value, t)
    if value ~= "" then
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
o:depends("dns_mode", "v2ray_doh")
o:depends("dns_mode", "xray_doh")
end

return m
