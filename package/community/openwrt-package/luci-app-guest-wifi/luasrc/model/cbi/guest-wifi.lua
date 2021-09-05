require("luci.tools.webadmin")

m = Map("guest-wifi", translate("Guest-wifi"))

s = m:section(TypedSection, "guest-wifi", translate("Config"), translate("You can set guest wifi here. The wifi will be disconnected when enabling/disabling. When modifying the password, first disable the guest wifi, and then do the modification, save and apply. Finally check both Enable and Create, save and apply."))
s.anonymous = true 
s.addremove = false

enable = s:option(Flag, "enable", translate("Enable"), translate("Enable or disable guest wifi"))
enable.default = false
enable.optional = false
enable.rmempty = false

create = s:option(Flag, "create", translate("Create/Remove"), translate("Check to create guest wifi when enabled, or check to remove guest wifi when disabled."))
create.default = false
create.optional = false
create.rmempty = false

device = s:option(ListValue, "device", translate("Define device"), translate("Define device of guest wifi"))
device:value("radio0", "radio0")
device:value("radio1", "radio1")
device:value("radio2", "radio2")
device:value("ra", "ra")
device:value("rai", "rai")
device.default = "radio0"

wifi_name = s:option(Value, "wifi_name", translate("Wifi name"), translate("Define the name of guest wifi"))
wifi_name.default = "Guest-WiFi"
wifi_name.rmempty = true

interface_name = s:option(Value, "interface_name", translate("Interface name"), translate("Define the interface name of guest wifi"))
interface_name.default = "guest"
interface_name.rmempty = true

interface_ip = s:option(Value, "interface_ip", translate("Interface IP address"), translate("Define IP address for guest wifi"))
interface_ip.datatype = "ip4addr"
interface_ip.default ="192.168.4.1"

encryption = s:option(Value, "encryption", translate("Encryption"), translate("Define encryption of guest wifi"))
encryption:value("psk", "WPA-PSK")
encryption:value("psk2", "WPA2-PSK")
encryption:value("none", "No Encryption")
encryption.default = "psk2"
encryption.widget = "select"

passwd = s:option(Value, "passwd", translate("Password"), translate("Define the password of guest wifi"))
passwd.password = true
passwd.default = "guestnetwork"

isolate = s:option(ListValue, "isolate", translate("Isolation"), translate("Enalbe or disable isolation"))
isolate:value("1", translate("YES"))
isolate:value("0", translate("NO"))

start = s:option(Value, "start", translate("Start address"), translate("Lowest leased address as offset from the network address"))
start.default = "50"
start.rmempty = true

limit = s:option(Value, "limit", translate("Client Limit"), translate("Maximum number of leased addresses"))
limit.default = "200"
limit.rmempty = true

leasetime = s:option(Value, "leasetime", translate("DHCP lease time"), translate("Expiry time of leased addresses, minimum is 2 minutes (2m)"))
leasetime.default = "1h"
leasetime.rmempty = true

return m

