
local ipc = require "luci.ip"
local sys = require "luci.sys"
local opkg = require "luci.model.ipkg"

local packageName = "wifidog-ng"
local m, s

if opkg.status(packageName)[packageName] then
    return Map(packageName, translate("WifiDog-ng"), translate('<b style="color:red">WifiDog-ng is not installed..</b>'))
end

m = Map("wifidog-ng", translate("WifiDog-ng"), translate("<a target=\"_blank\" href=\"https://github.com/zhaojh329/wifidog-ng\">WifiDog-ng</a> It is a very efficient solution for wireless hotspot authentication."))

m:section(SimpleSection).template  = "wifidog-ng/wifidog-ng_status"

s = m:section(TypedSection, "gateway", translate("Client Settings"))
s.anonymous = true
s.addremove = false

s:tab("general", translate("General Settings"))
s:tab("advanced", translate("Advanced Settings"))

-- Client Settings
Enabled = s:taboption("general", Flag, "enabled", translate("Enabled"),translate(""))
Enabled.rmempty = false
Enabled.default = "1"

DhcpHostWhite = s:taboption("general", Flag, "dhcp_host_white", translate("Enabled Trusted DHCP MAC"),translate("Does not support 1.5.6 and below"))
DhcpHostWhite.rmempty = false
DhcpHostWhite.default = "1"

Id = s:taboption("general",Value, "id", translate("Gateway ID"), translate("The mac address of the default GatewayInterface"))
Id.placeholder = luci.util.exec("ifconfig br-lan| grep HWaddr | awk -F \" \" '{print $5}' | awk '$1~//{print;exit}' | sed 's/://g'")


GatewayInterface = s:taboption("general", Value, "ifname", translate("Gateway Interface"), translate("Set this to the internal IP address of the gateway, default 'br-lan'"))
GatewayInterface.default = "br-lan"
for _, e in ipairs(sys.net.devices()) do
    if e ~= "lo" then GatewayInterface:value(e) end
end

Port = s:taboption("advanced", Value, "port", translate("Gateway Port"), translate("Listen HTTP on this port"))
Port.datatype = "port"

SSLPort = s:taboption("advanced", Value, "ssl_port", translate("Gateway SSLPort"), translate("Listen HTTPS on this port"))
SSLPort.datatype = "port"

s:taboption("advanced", Value, "ssid", translate("WeChat SSID"), translate("WeChat Use this ssid"))

CheckInterval = s:taboption("advanced", Value, "checkinterval", translate("Check Interval"), translate("How many seconds should we wait between timeout checks."))
CheckInterval.datatype = "uinteger"
CheckInterval.default = "30"

ClientTimeout = s:taboption("advanced", Value, "client_timeout", translate("Client Timeout"), translate("Set this to the desired of number of CheckInterval of inactivity before a client is logged out. The timeout will be INTERVAL * TIMEOUT"))
ClientTimeout.datatype = "uinteger"
ClientTimeout.default = "5"

TemppassTime = s:taboption("advanced", Value, "temppass_time", translate("Temppass Time"), translate("Allow users to pass in a number of seconds"))
TemppassTime.datatype = "uinteger"
TemppassTime.default = "30"

-- Server Settings
s = m:section(TypedSection, "authserver", translate("Server Settings"))
s.anonymous = true
s.addremove = false

s:tab("general", translate("General Settings"))
s:tab("advanced", translate("Advanced Settings"))

s:taboption("general", Value, "host", translate("AuthServer Hostname"), translate("AuthServer Hostname Or IP"))

Path = s:taboption("general", Value, "path", translate("AuthServer Path"), translate("The path must be both prefixed and suffixed by /. Use a single / for server root."))
Path.default = "/wifidog/"

ServerPort = s:taboption("general", Value, "port", translate("AuthServer Port"), translate(""))
ServerPort.datatype = "port"

ServerSSL = s:taboption("general", Flag, "ssl", translate("SSL Available"),translate("Use SSL"))
ServerSSL.rmempty = false
ServerSSL.default = "0"

LoginPath = s:taboption("advanced", Value, "login_path", translate("Login ScriptPath"), translate("This is the script the user will be sent to for login."))
LoginPath.default = "login"

PortalPath = s:taboption("advanced", Value, "portal_path", translate("Portal ScriptPath"), translate("This is the script the user will be sent to after a successfull login."))
PortalPath.default = "portal"

MsgPath = s:taboption("advanced", Value, "msg_path", translate("Msg ScriptPath"), translate("This is the script the user will be sent to upon error to read a readable message."))
MsgPath.default = "gw_message.php"

PingPath = s:taboption("advanced", Value, "ping_path", translate("Ping ScriptPath"), translate("This is the script the user will be sent to check server."))
PingPath.default = "ping"

AuthPath = s:taboption("advanced", Value, "auth_path", translate("Auth ScriptPath"), translate("This is the script the user will be sent to check auth."))
AuthPath.default = "auth"

-- Trusted MAC List
s = m:section(TypedSection,"whitelist_mac",translate("Trusted MAC List"), translate("MAC addresses who are allowed to pass through without authentication."))
s.template = "cbi/tblsection"
s.anonymous = true
s.addremove = true

e = s:option(Value, "name", translate("Hostname"))

mac = s:option(Value, "mac", translate("<abbr title=\"Media Access Control\">MAC</abbr>-Address"))
mac.datatype = "list(macaddr)"
mac.rmempty  = true

function mac.cfgvalue(self, section)
    local val = Value.cfgvalue(self, section)
    return ipc.checkmac(val) or val
end

sys.net.host_hints(function(m, v4, v6, name)
    if m and v4 then
        mac:value(m, "%s (%s)" %{ m, name or v4 })
    end
end)

-- Trusted Domain List
s = m:section(TypedSection,"whitelist_domain",translate("Trusted Domain List"),translate(""))
s.template = "cbi/tblsection"
s.anonymous = true
s.addremove = true

s:option(Value,"domain",translate("Domain Or IP"))


m:section(SimpleSection).template = "wifidog-ng/client_list"

return m