-- Copyright 2015 Daniel Dickinson <openwrt@daniel.thecshore.com>
-- Licensed to the public under the Apache License 2.0.

local map, section, net = ...

local server, username, password, hexpassword
local authgroup, interface, passgroup, hexpassgroup
local domain, vendor, natt_mode, dh_group
local pfs, enable_single_des, enable_no_enc
local mtu, local_addr, local_port, dpd_idle
local auth_mode, target_network

local ifc = net:get_interface():name()

server = section:taboption("general", Value, "server", translate("VPN Server"))
server.datatype = "host(0)"

port = section:taboption("general", Value, "local_addr", translate("VPN Local address"))
port.placeholder = "0.0.0.0"
port.datatype    = "ipaddr"

port = section:taboption("general", Value, "local_port", translate("VPN Local port"))
port.placeholder = "500"
port.datatype    = "port"

ifname = section:taboption("general", Value, "interface", translate("Output Interface"))
ifname.template = "cbi/network_netlist"

mtu = section:taboption("general", Value, "mtu", translate("MTU"))
mtu.datatype = "uinteger"

username = section:taboption("general", Value, "username", translate("Username"))
password = section:taboption("general", Value, "password", translate("Password"))
password.password = true
hexpassword = section:taboption("general", Value, "hexpassword", translate("Obfuscated Password"))
hexpassword.password = true
authroup = section:taboption("general", Value, "authgroup", translate("Auth Group"))
passgroup = section:taboption("general", Value, "passgroup", translate("Group Password"))
passgroup.password = true
hexpassgroup = section:taboption("general", Value, "hexpassgroup", translate("Obfuscated Group Password"))
hexpassword.password= true

domain = section:taboption("general", Value, "domain", translate("NT Domain"))
vendor = section:taboption("general", Value, "vendor", translate("Vendor"))
dh_group = section:taboption("general", ListValue, "dh_group", translate("IKE DH Group"))
dh_group:value("dh2")
dh_group:value("dh1")
dh_group:value("dh5")

pfs = section:taboption("general", ListValue, "pfs", translate("Perfect Forward Secrecy"))
pfs:value("server")
pfs:value("nopfs")
pfs:value("dh1")
pfs:value("dh2")
pfs:value("dh5")

natt_mode = section:taboption("general", ListValue, "natt_mode", translate("NAT-T Mode"))
natt_mode:value("natt", translate("RFC3947 NAT-T mode"))
natt_mode:value("none", translate("No NAT-T"))
natt_mode:value("force-natt", translate("Force use of NAT-T"))
natt_mode:value("cisco-udp", translate("Cisco UDP encapsulation"))

enable_no_enc = section:taboption("general", Flag, "enable_no_enc",
	translate("Disable Encryption"),
	translate("If checked, encryption is disabled"))
enable_no_enc.default = enable_no_enc.disabled

enable_single_des = section:taboption("general", Flag, "enable_single_des",
	translate("Enable Single DES"),
	translate("If checked, 1DES is enabled"))
enable_no_enc.default = enable_single_des.disabled

dpd_idle = section:taboption("general", Value, "dpd_idle", translate("DPD Idle Timeout"))
dpd_idle.datatype = "uinteger"
dpd_idle.placeholder = "600"

ifname = section:taboption("general", Value, "target_network", translate("Target network"))
port.placeholder = "0.0.0.0/0"
port.datatype    = "network"
