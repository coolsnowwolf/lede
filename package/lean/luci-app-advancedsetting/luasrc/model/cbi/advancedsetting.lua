#-- Copyright (C) 2018 dz <dingzhong110@gmail.com>
local fs = require "nixio.fs"
local sys = require "luci.sys"
m = Map("advancedsetting", translate("Advanced Setting"), translate("Direct editing of built-in Script Documents for various services, unless you know what you are doing, do not easily modify these configuration documents"))
s = m:section(TypedSection, "advancedsetting")
s.anonymous=true
--dnsmasq
if nixio.fs.access("/etc/dnsmasq.conf") then
s:tab("config1", translate("dnsmasq"),translate("This page is the document content for configuring /etc/dnsmasq.conf. Restart takes effect after application is saved"))
conf = s:taboption("config1", Value, "editconf1", nil, translate("Each line of the opening numeric symbol (#) or semicolon (;) is considered a comment; delete (;) and enable the specified option."))
conf.template = "cbi/tvalue"
conf.rows = 20
conf.wrap = "off"
function conf.cfgvalue(self, section)
    return fs.readfile("/etc/dnsmasq.conf") or ""
end
function conf.write(self, section, value)
    if value then
        value = value:gsub("\r\n?", "\n")
        fs.writefile("/tmp/dnsmasq.conf", value)
        if (luci.sys.call("cmp -s /tmp/dnsmasq.conf /etc/dnsmasq.conf") == 1) then
            fs.writefile("/etc/dnsmasq.conf", value)
            luci.sys.call("/etc/init.d/dnsmasq restart >/dev/null")
        end
        fs.remove("/tmp/dnsmasq.conf")
    end
end
end
--network
if nixio.fs.access("/etc/config/network") then
s:tab("config2", translate("network"),translate("This page is the document content for configuring /etc/config/network. Restart takes effect after application is saved"))
conf = s:taboption("config2", Value, "editconf2", nil, translate("Each line of the opening numeric symbol (#) or semicolon (;) is considered a comment; delete (;) and enable the specified option."))
conf.template = "cbi/tvalue"
conf.rows = 20
conf.wrap = "off"
function conf.cfgvalue(self, section)
    return fs.readfile("/etc/config/network") or ""
end
function conf.write(self, section, value)
    if value then
        value = value:gsub("\r\n?", "\n")
        fs.writefile("/tmp/netwok", value)
        if (luci.sys.call("cmp -s /tmp/network /etc/config/network") == 1) then
            fs.writefile("/etc/config/network", value)
            luci.sys.call("/etc/init.d/network restart >/dev/null")
        end
        fs.remove("/tmp/network")
    end
end
end

--hosts
if nixio.fs.access("/etc/hosts") then
s:tab("config3", translate("hosts"),translate("This page is the document content for configuring /etc/hosts. Restart takes effect after application is saved"))
conf = s:taboption("config3", Value, "editconf3", nil, translate("Each line of the opening numeric symbol (#) or semicolon (;) is considered a comment; delete (;) and enable the specified option."))
conf.template = "cbi/tvalue"
conf.rows = 20
conf.wrap = "off"
function conf.cfgvalue(self, section)
    return fs.readfile("/etc/hosts") or ""
end
function conf.write(self, section, value)
    if value then
        value = value:gsub("\r\n?", "\n")
        fs.writefile("/tmp/etc/hosts", value)
        if (luci.sys.call("cmp -s /tmp/etc/hosts /etc/hosts") == 1) then
            fs.writefile("/etc/hosts", value)
            luci.sys.call("/etc/init.d/dnsmasq restart >/dev/null")
        end
        fs.remove("/tmp/etc/hosts")
    end
end
end

return m
