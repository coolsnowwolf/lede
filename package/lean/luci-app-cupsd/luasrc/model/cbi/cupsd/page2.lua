local fs = require "nixio.fs"
local sys = require "luci.sys"

m = Map("samba", translate("<font color=\"green\">CUPS高级配置</font>"), translate("<font color=\"red\">此处修改的是/etc/cups/cupsd.conf文件。</font><input class=\"cbi-button cbi-button-apply\" type=\"submit\" value=\" "..translate("点击此处可下载并浏览添加打印机教程文件").." \" onclick=\"window.open('http://'+window.location.hostname+'/cups.pdf')\"/>"))
s = m:section(TypedSection, "samba")
s.anonymous=true

o = s:option(TextValue, "/etc/cups/cupsd.conf")
o.rows = 20
o.wrap = "off"
function o.cfgvalue(self, section)
    return fs.readfile("/etc/cups/cupsd.conf") or ""
end

function o.write(self, section, value)
    if value then
        value = value:gsub("\r\n?", "\n")
        fs.writefile("/tmp/cupsd.conf", value)
        if (luci.sys.call("cmp -s /tmp/cupsd.conf /etc/cups/cupsd.conf") == 1) then
            fs.writefile("/etc/cups/cupsd.conf", value)
            luci.sys.call("/etc/init.d/cupsd reload >/dev/null")
        end
        fs.remove("/tmp/cupsd.conf")
    end
end

return m


