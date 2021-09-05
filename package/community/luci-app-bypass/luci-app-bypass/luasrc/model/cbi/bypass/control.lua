local fs = require "nixio.fs"
local m,s,o

m=Map("bypass",translate("IP Access Control"))
s=m:section(TypedSection,"access_control")
s.anonymous=true

m.apply_on_parse=true
function m.on_apply(self)
luci.sys.call("/etc/init.d/bypass reload > /dev/null 2>&1 &")
end

s:tab("wan_ac",translate("WAN IP AC"))

o=s:taboption("wan_ac",DynamicList,"wan_bp_ips",translate("WAN White List IP"))
o.datatype="ip4addr"

o=s:taboption("wan_ac",DynamicList,"wan_fw_ips",translate("WAN Force Proxy IP"))
o.datatype="ip4addr"

s:tab("lan_ac",translate("LAN IP AC"))

o=s:taboption("lan_ac",ListValue,"lan_ac_mode",translate("LAN Access Control"))
o:value("w",translate("Allow listed only"))
o:value("b",translate("Allow all except listed"))
o.rmempty=false

o=s:taboption("lan_ac",DynamicList,"lan_ac_ips",translate("LAN Host List"))
o.datatype="ipaddr"
luci.ip.neighbors({family=4},function(entry)
	if entry.reachable then
		o:value(entry.dest:string())
	end
end)
o:depends("lan_ac_mode","w")
o:depends("lan_ac_mode","b")

o=s:taboption("lan_ac",DynamicList,"lan_fp_ips",translate("LAN Force Proxy Host List"))
o.datatype="ipaddr"
luci.ip.neighbors({family=4},function(entry)
	if entry.reachable then
		o:value(entry.dest:string())
	end
end)

o=s:taboption("lan_ac",DynamicList,"lan_gm_ips",translate("Game Mode Host List"))
o.datatype="ipaddr"
luci.ip.neighbors({family=4},function(entry)
	if entry.reachable then
		o:value(entry.dest:string())
	end
end)

s:tab("direct_list", translate("Direct Domain List"))
s:tab("proxy_list", translate("Proxy Domain List"))
s:tab("netflix_list", translate("Netflix Domain List"))
s:tab("oversea_list", translate("Oversea Domain List"))

---- Direct Hosts
local direct_host = string.format("/etc/bypass/white.list")
o = s:taboption("direct_list", TextValue, "direct_host", "", "<font color='red'>" .. translate("These had been joined websites will not proxy.") .. "</font>")
o.rows = 15
o.wrap = "off"
o.cfgvalue = function(self, section) return fs.readfile(direct_host) or "" end
o.write = function(self, section, value) fs.writefile(direct_host, value:gsub("\r\n", "\n")) end
o.remove = function(self, section, value) fs.writefile(direct_host, "") end
o.validate = function(self, value)
    local hosts= {}
    string.gsub(value, '[^' .. "\r\n" .. ']+', function(w) table.insert(hosts, w) end)
    for index, host in ipairs(hosts) do
        if not datatypes.hostname(host) then
            return nil, host .. " " .. translate("Not valid domain name, please re-enter!")
        end
    end
    return value
end

---- Proxy Hosts
local proxy_host = string.format("/etc/bypass/black.list")
o = s:taboption("proxy_list", TextValue, "proxy_host", "", "<font color='red'>" .. translate("These had been joined websites will use proxy.") .. "</font>")
o.rows = 15
o.wrap = "off"
o.cfgvalue = function(self, section) return fs.readfile(proxy_host) or "" end
o.write = function(self, section, value) fs.writefile(proxy_host, value:gsub("\r\n", "\n")) end
o.remove = function(self, section, value) fs.writefile(proxy_host, "") end
o.validate = function(self, value)
    local hosts= {}
    string.gsub(value, '[^' .. "\r\n" .. ']+', function(w) table.insert(hosts, w) end)
    for index, host in ipairs(hosts) do
        if not datatypes.hostname(host) then
            return nil, host .. " " .. translate("Not valid domain name, please re-enter!")
        end
    end
    return value
end

---- Netflix Hosts
local netflix_host = string.format("/etc/bypass/netflix.list")
o = s:taboption("netflix_list", TextValue, "netflix_host", "", "<font color='red'>" .. translate("Netflix Domain List") .. "</font>")
o.rows = 15
o.wrap = "off"
o.cfgvalue = function(self, section) return fs.readfile(netflix_host) or "" end
o.write = function(self, section, value) fs.writefile(netflix_host, value:gsub("\r\n", "\n")) end
o.remove = function(self, section, value) fs.writefile(netflix_host, "") end
o.validate = function(self, value)
    local hosts= {}
    string.gsub(value, '[^' .. "\r\n" .. ']+', function(w) table.insert(hosts, w) end)
    for index, host in ipairs(hosts) do
        if not datatypes.hostname(host) then
            return nil, host .. " " .. translate("Not valid domain name, please re-enter!")
        end
    end
    return value
end

---- Oversea Hosts
local oversea_host = string.format("/etc/bypass/oversea.list")
o = s:taboption("oversea_list", TextValue, "oversea_host", "", "<font color='red'>" .. translate("Oversea Domain List") .. "</font>")
o.rows = 15
o.wrap = "off"
o.cfgvalue = function(self, section) return fs.readfile(oversea_host) or "" end
o.write = function(self, section, value) fs.writefile(oversea_host, value:gsub("\r\n", "\n")) end
o.remove = function(self, section, value) fs.writefile(oversea_host, "") end
o.validate = function(self, value)
    local hosts= {}
    string.gsub(value, '[^' .. "\r\n" .. ']+', function(w) table.insert(hosts, w) end)
    for index, host in ipairs(hosts) do
        if not datatypes.hostname(host) then
            return nil, host .. " " .. translate("Not valid domain name, please re-enter!")
        end
    end
    return value
end

return m
