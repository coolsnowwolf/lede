-- Copyright (C) 2017 fuyumi <280604399@qq.com>
-- Licensed to the public under the GNU Affero General Public License v3.

local m, s

local running = (luci.sys.call("pidof dogcom > /dev/null") == 0)
if running then 
    m = Map("dogcom", translate("Dogcom config"), translate("Dogcom is running."))
else
    m = Map("dogcom", translate("Dogcom config"), translate("Dogcom is not running."))
end

s = m:section(TypedSection, "dogcom", "")
s.addremove = false
s.anonymous = true

-- Basic Settings --
s:tab("basic", translate("Basic Settings"))

enable = s:taboption("basic", Flag, "enabled", translate("Enable"))
enable.rmempty = false
function enable.cfgvalue(self, section)
    return luci.sys.init.enabled("dogcom") and self.enabled or self.disabled
end

version = s:taboption("basic", ListValue, "version", translate("Version"))
version:value("dhcp", translate("dhcp"))
version:value("pppoe", translate("pppoe"))
version.value = "dhcp"

escpatch = s:taboption("basic", Button, "esc", translate("Patch the escape problem"))
function escpatch.write()
    luci.sys.call("sed -i '/proto_run_command/i username=`echo -e \"$username\"`' /lib/netifd/proto/ppp.sh")
    luci.sys.call("sed -i '/proto_run_command/i password=`echo -e \"$password\"`' /lib/netifd/proto/ppp.sh")
end

config = s:taboption("basic", Value, "config", translate("configfile"), translate("This file is /etc/dogcom.conf."), "")
config.template = "cbi/tvalue"
config.rows = 15
config.wrap = "off"

function config.cfgvalue(self, section)
    return nixio.fs.readfile("/etc/dogcom.conf")
end

function config.write(self, section, value)
    value = value:gsub("\r\n?", "\n")
    nixio.fs.writefile("/etc/dogcom.conf", value)
end

-- Generate Configuration --
s:tab("generator", translate("Generate Configuration"))

msg = s:taboption("generator", DummyValue, "", translate(""), 
translate("Please upload your packet file. Then change password and copy to basic settings."))

autoconfig = s:taboption("generator", DummyValue, "autoconfig")
autoconfig.template = "dogcom/auto_configure"

-- Save Configuration --
function enable.write(self, section, value)
    if value == "1" then
        luci.sys.call("/etc/init.d/dogcom enable >/dev/null")
        luci.sys.call("/etc/init.d/dogcom start >/dev/null")
    else
        luci.sys.call("/etc/init.d/dogcom stop >/dev/null")
        luci.sys.call("/etc/init.d/dogcom disable >/dev/null")
    end
    Flag.write(self, section, value)
end

return m