local function is_running(name)
    if luci.sys.call("pidof %s >/dev/null" %{name}) == 0 then
        return translate("RUNNING")
    else
        return translate("NOT RUNNING")
    end
end

local function is_online(ipaddr)
    if ipaddr == "0.0.0.0" then 
        return translate("Pinghost not set")
    end
    if luci.sys.call("ping -c1 -w1 %s >/dev/null 2>&1" %{ipaddr}) == 0 then
        return translate("ONLINE")
    else
        return translate("NOT ONLINE")
    end
end

require("luci.sys")

m = Map("mentohust", translate("MentoHUST"), translate("Configure MentoHUST 802.11x."))

s = m:section(TypedSection, "mentohust", translate("Status"))
s.anonymous = true
status = s:option(DummyValue,"_mentohust_status", "MentoHUST")
status.value = "<span id=\"_mentohust_status\">%s</span>" %{is_running("mentohust")}
status.rawhtml = true
t = io.popen('uci get mentohust.@mentohust[0].pinghost')
netstat=is_online(tostring(t:read("*line")))
t:close()
if netstat ~= "" then
netstatus = s:option(DummyValue,"_network_status", translate("Network Status"))
netstatus.value = "<span id=\"_network_status\">%s</span>" %{netstat}
netstatus.rawhtml = true
end

o = m:section(TypedSection, "mentohust", translate("Settings"))
o.addremove = false
o.anonymous = true

o:tab("base", translate("Normal Settings"))
o:tab("advanced", translate("Advanced Settings"))

enable = o:taboption("base", Flag, "enable", translate("Enable"))
name = o:taboption("base", Value, "username", translate("Username"),translate("The username given to you by your network administrator"))
pass = o:taboption("base", Value, "password", translate("Password"), translate("The password you set or given to you by your network administrator"))
pass.password = true

ifname = o:taboption("base", ListValue, "ifname", translate("Interfaces"), translate("Physical interface of WAN"))
for k, v in ipairs(luci.sys.net.devices()) do
    if v ~= "lo" then
        ifname:value(v)
    end
end

pinghost = o:taboption("base", Value, "pinghost", translate("PingHost"), translate("Ping host for drop detection, 0.0.0.0 to turn off this feature"))
pinghost.default = "0.0.0.0"

ipaddr = o:taboption("advanced", Value, "ipaddr", translate("IP Address"), translate("Your IPv4 Address. (DHCP users can set to 0.0.0.0)"))
ipaddr.default = "0.0.0.0"

mask = o:taboption("advanced", Value, "mask", translate("NetMask"), translate("NetMask, it doesn't matter"))
mask.default = "0.0.0.0"

gateway = o:taboption("advanced", Value, "gateway", translate("Gateway"), translate("Gateway, if specified, will monitor gateway ARP information"))
gateway.default = "0.0.0.0"

dnsserver = o:taboption("advanced", Value, "dns", translate("DNS server"), translate("DNS server, it doesn't matter"))
dnsserver.default = "0.0.0.0"

timeout = o:taboption("advanced", Value, "timeout", translate("Timeout"), translate("Each timeout of the package (seconds)"))
timeout.default = "8"

echointerval = o:taboption("advanced", Value, "echointerval", translate("EchoInterval"), translate("Interval for sending Echo packets (seconds)"))
echointerval.default = "30"

restartwait = o:taboption("advanced", Value, "restartwait", translate("RestartWait"), translate("Failed Wait (seconds) Wait for seconds after authentication failed or restart authentication after server request"))
restartwait.default = "15"

startmode = o:taboption("advanced", ListValue, "startmode", translate("StartMode"), translate("Multicast address type when searching for servers"))
startmode:value(0, translate("Standard"))
startmode:value(1, translate("Ruijie"))
startmode:value(2, translate("Uses MentoHUST for Xaar certification"))
startmode.default = "0"

dhcpmode = o:taboption("advanced", ListValue, "dhcpmode", translate("DhcpMode"), translate("DHCP method"))
dhcpmode:value(0, translate("None"))
dhcpmode:value(1, translate("secondary authentication"))
dhcpmode:value(2, translate("after certification"))
dhcpmode:value(3, translate("before certification"))
dhcpmode.default = "2"

shownotify = o:taboption("advanced", Value, "shownotify", translate("ShowNotify"), translate("Whether to display notifications 0 (no) 1 to 20 (yes)"))
shownotify.default = "5"

version = o:taboption("advanced", Value, "version", translate("Client Version"), translate("Client version number. If client verification is not enabled but the version number is required, it can be specified here. The format is 3.30."))
version.default = "0.00"

datafile = o:taboption("advanced", Value, "datafile", translate("DataFile"), translate("Authentication data file, if you need to verify the client, you need to set correctly"))
datafile.default = "/etc/mentohust/"

dhcpscript = o:taboption("advanced", Value, "dhcpscript", translate("DhcpScript"), translate("DHCP script"))
dhcpscript.default = "udhcpc -i"

local apply = luci.http.formvalue("cbi.apply")
if apply then
    io.popen("/etc/init.d/mentohust restart")
end

return m
