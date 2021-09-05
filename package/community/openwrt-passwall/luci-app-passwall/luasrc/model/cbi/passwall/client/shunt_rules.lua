local api = require "luci.model.cbi.passwall.api.api"
local appname = api.appname
local datatypes = api.datatypes

m = Map(appname, "V2ray/Xray" .. translate("Shunt") .. translate("Rule"))
m.redirect = api.url()

s = m:section(NamedSection, arg[1], "shunt_rules", "")
s.addremove = false
s.dynamic = false

remarks = s:option(Value, "remarks", translate("Remarks"))
remarks.default = arg[1]
remarks.rmempty = false

protocol = s:option(MultiValue, "protocol", translate("Protocol"))
protocol:value("http")
protocol:value("tls")
protocol:value("bittorrent")

domain_list = s:option(TextValue, "domain_list", translate("Domain"))
domain_list.rows = 10
domain_list.wrap = "off"
domain_list.validate = function(self, value)
    local hosts= {}
    string.gsub(value, '[^' .. "\r\n" .. ']+', function(w) table.insert(hosts, w) end)
    for index, host in ipairs(hosts) do
        local flag = 1
        local tmp_host = host
        if host:find("regexp:") and host:find("regexp:") == 1 then
            flag = 0
        elseif host:find("domain:.") and host:find("domain:.") == 1 then
            tmp_host = host:gsub("domain:", "")
        elseif host:find("full:.") and host:find("full:.") == 1 then
            tmp_host = host:gsub("full:", "")
        elseif host:find("geosite:") and host:find("geosite:") == 1 then
            flag = 0
        elseif host:find("ext:") and host:find("ext:") == 1 then
            flag = 0
        end
        if flag == 1 then
            if not datatypes.hostname(tmp_host) then
                return nil, tmp_host .. " " .. translate("Not valid domain name, please re-enter!")
            end
        end
    end
    return value
end
domain_list.description = "<br /><ul><li>" .. translate("Plaintext: If this string matches any part of the targeting domain, this rule takes effet. Example: rule 'sina.com' matches targeting domain 'sina.com', 'sina.com.cn' and 'www.sina.com', but not 'sina.cn'.")
.. "</li><li>" .. translate("Regular expression: Begining with 'regexp:', the rest is a regular expression. When the regexp matches targeting domain, this rule takes effect. Example: rule 'regexp:\\.goo.*\\.com$' matches 'www.google.com' and 'fonts.googleapis.com', but not 'google.com'.")
.. "</li><li>" .. translate("Subdomain (recommended): Begining with 'domain:' and the rest is a domain. When the targeting domain is exactly the value, or is a subdomain of the value, this rule takes effect. Example: rule 'domain:v2ray.com' matches 'www.v2ray.com', 'v2ray.com', but not 'xv2ray.com'.")
.. "</li><li>" .. translate("Full domain: Begining with 'full:' and the rest is a domain. When the targeting domain is exactly the value, the rule takes effect. Example: rule 'domain:v2ray.com' matches 'v2ray.com', but not 'www.v2ray.com'.")
.. "</li><li>" .. translate("Pre-defined domain list: Begining with 'geosite:' and the rest is a name, such as geosite:google or geosite:cn.")
.. "</li><li>" .. translate("Domains from file: Such as 'ext:file:tag'. The value must begin with ext: (lowercase), and followed by filename and tag. The file is placed in resource directory, and has the same format of geosite.dat. The tag must exist in the file.")
.. "</li></ul>"
ip_list = s:option(TextValue, "ip_list", "IP")
ip_list.rows = 10
ip_list.wrap = "off"
ip_list.validate = function(self, value)
    local ipmasks= {}
    string.gsub(value, '[^' .. "\r\n" .. ']+', function(w) table.insert(ipmasks, w) end)
    for index, ipmask in ipairs(ipmasks) do
        if ipmask:find("geoip:") and ipmask:find("geoip:") == 1 then
        elseif ipmask:find("ext:") and ipmask:find("ext:") == 1 then
        else
            if not (datatypes.ipmask4(ipmask) or datatypes.ipmask6(ipmask)) then
                return nil, ipmask .. " " .. translate("Not valid IP format, please re-enter!")
            end
        end
    end
    return value
end
ip_list.description = "<br /><ul><li>" .. translate("IP: such as '127.0.0.1'.")
.. "</li><li>" .. translate("CIDR: such as '127.0.0.0/8'.")
.. "</li><li>" .. translate("GeoIP: such as 'geoip:cn'. It begins with geoip: (lower case) and followed by two letter of country code.")
.. "</li><li>" .. translate("IPs from file: Such as 'ext:file:tag'. The value must begin with ext: (lowercase), and followed by filename and tag. The file is placed in resource directory, and has the same format of geoip.dat. The tag must exist in the file.")
.. "</li></ul>"

return m
