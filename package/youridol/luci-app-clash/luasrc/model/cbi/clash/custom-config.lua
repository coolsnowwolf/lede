-- Copyright (C) 2019 Anton Chen <contact@antonchen.com>
local nxfs = require "nixio.fs"
local http = require "luci.http"

local custom_proxy_file = "/etc/clash/custom-proxy"
b = SimpleForm("custom_proxy", nil, translate("Custom proxy"))
p = b:field(TextValue, "custom_proxy")
p.wrap = "off"
p.rows = 6
p.cfgvalue = function(self, section)
    return nxfs.readfile(custom_proxy_file) or ""
end
p.write = function(self, section, data)
    return nxfs.writefile(custom_proxy_file, data:gsub("\r\n", "\n"))
end
b.handle = function(self, state, data)
    return true
end

local custom_group_file = "/etc/clash/custom-group"
c = SimpleForm("custom_group", nil, translate("Custom group"))
g = c:field(TextValue, "custom_group")
g.wrap = "off"
g.rows = 6
g.cfgvalue = function(self, section)
    return nxfs.readfile(custom_group_file) or ""
end
g.write = function(self, section, data)
    return nxfs.writefile(custom_group_file, data:gsub("\r\n", "\n"))
end
c.handle = function(self, state, data)
    return true
end

local custom_select_file = "/etc/clash/custom-select"
d = SimpleForm("custom_select", nil, translate("Custom group select"))
s = d:field(TextValue, "custom_select")
s.wrap = "off"
s.rows = 6
s.cfgvalue = function(self, section)
    return nxfs.readfile(custom_select_file) or ""
end
s.write = function(self, section, data)
    return nxfs.writefile(custom_select_file, data:gsub("\r\n", "\n"))
end
d.handle = function(self, state, data)
    return true
end

local custom_rule_file = "/etc/clash/custom-rule"
e = SimpleForm("custom_rule", nil, translate("Custom rule"))
r = e:field(TextValue, "custom_rule")
r.wrap = "off"
r.rows = 6
r.cfgvalue = function(self, section)
    return nxfs.readfile(custom_rule_file) or ""
end
r.write = function(self, section, data)
    return nxfs.writefile(custom_rule_file, data:gsub("\r\n", "\n"))
end
e.handle = function(self, state, data)
    return true
end

--- Submit
local submit = http.formvalue("cbi.submit")
if submit then
    os.execute("sleep 5;/etc/init.d/clash restart >/dev/null 2>&1 &")
end

return b, c, d, e
