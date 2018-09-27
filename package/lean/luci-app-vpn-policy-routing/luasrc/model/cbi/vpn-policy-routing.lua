-- local readmeURL = "https://github.com/openwrt/packages/tree/master/net/vpn-policy-routing/files/README.md"
local readmeURL = "https://github.com/stangri/openwrt_packages/tree/master/vpn-policy-routing/files/README.md"

-- function log(obj)
-- 	if obj ~= nil then if type(obj) == "table" then luci.util.dumptable(obj) else luci.util.perror(obj) end else luci.util.perror("Empty object") end
-- end

local uci = require "luci.model.uci".cursor()
local sys = require "luci.sys"
local util = require "luci.util"
local ip = require "luci.ip"
local fs = require "nixio.fs"

local t = uci:get("vpn-policy-routing", "config", "supported_interface")
if not t then
	supportedIfaces = ""
elseif type(t) == "table" then
	for key,value in pairs(t) do supportedIfaces = supportedIfaces and supportedIfaces .. ' ' .. value or value end
elseif type(t) == "string" then
	supportedIfaces = t
end

t = uci:get("vpn-policy-routing", "config", "ignored_interface")
if not t then
	ignoredIfaces = ""
elseif type(t) == "table" then
	for key,value in pairs(t) do ignoredIfaces = ignoredIfaces and ignoredIfaces .. ' ' .. value or value end
elseif type(t) == "string" then
	ignoredIfaces = t
end

local lanIPAddr = uci:get("network", "lan", "ipaddr")
local lanNetmask = uci:get("network", "lan", "netmask")
if lanIPAddr and lanNetmask then
	laPlaceholder = ip.new(lanIPAddr .. "/" .. lanNetmask )
end

function is_supported_interface(arg)
	local name=arg['.name']
	local proto=arg['proto']
	local ifname=arg['ifname']

	if name and supportedIfaces:find(name) then return true end
	if name and not ignoredIfaces:find(name) then
		if type(ifname) == "table" then
			for key,value in pairs(ifname) do
				if value and value:sub(1,3) == "tun" then return true end
				if value and value:sub(1,3) == "tap" then return true end
				if value and value:sub(1,3) == "tor" then return true end
				if value and fs.access("/sys/devices/virtual/net/" .. value .. "/tun_flags") then return true end
			end
		elseif type(ifname) == "string" then
			if ifname and ifname:sub(1,3) == "tun" then return true end
			if ifname and ifname:sub(1,3) == "tap" then return true end
			if ifname and ifname:sub(1,3) == "tor" then return true end
			if ifname and fs.access("/sys/devices/virtual/net/" .. ifname .. "/tun_flags") then return true end
		end
		if proto and proto:sub(1,11) == "openconnect" then return true end
		if proto and proto:sub(1,4) == "pptp" then return true end
		if proto and proto:sub(1,4) == "l2tp" then return true end
		if proto and proto:sub(1,9) == "wireguard" then return true end
	end
end

c = Map("vpn-policy-routing", translate("Openconnect, OpenVPN, PPTP, Wireguard and WAN Policy-Based Routing"))
h = c:section(NamedSection, "config", "vpn-policy-routing", translate("Service Status"))
local packageName = "vpn-policy-routing"
local enabledFlag = uci:get(packageName, "config", "enabled")
local status = util.ubus('service', 'list', { name = packageName })
if status and status[packageName] and status[packageName]['instances'] and status[packageName]['instances']['status'] and status[packageName]['instances']['status']['data'] and status[packageName]['instances']['status']['data']['status'] then
	status = status[packageName]['instances']['status']['data']['status']
else
	local ipt_status = util.trim(sys.exec("iptables-save | grep -m1 'VPR_PREROUTING'"))
	status = util.trim(sys.exec("/bin/ubus call service list \"{'name': 'vpn-policy-routing'}\" | /usr/bin/jsonfilter -l1 -e \"@['vpn-policy-routing']['instances']['status']['data']['status']\""))
	if not status or status == "" then
		if ipt_status and ipt_status ~= "" then
			status = "Started without PROCD support"
		else
			status = "Stopped"
		end
	end
end
en = h:option(Button, "__toggle")
if enabledFlag ~= "1" or status:match("Stopped") then
	en.title      = translate("Service is disabled/stopped")
	en.inputtitle = translate("Enable/Start")
	en.inputstyle = "apply important"
else
	en.title      = translate("Service is enabled/started")
	en.inputtitle = translate("Stop/Disable")
	en.inputstyle = "reset important"
	ds = h:option(DummyValue, "_dummy", translate("Service Status"))
	ds.template = "vpn-policy-routing/status"
	ds.value = status
	if not status:match("Success") and not status:match("Started without PROCD support") then
		reload = h:option(Button, "__reload")
		reload.title      = translate("Service started with error(s)")
		reload.inputtitle = translate("Reload")
		reload.inputstyle = "apply important"
		function reload.write()
			luci.sys.exec("/etc/init.d/vpn-policy-routing reload")
			luci.http.redirect(luci.dispatcher.build_url("admin/services/" .. packageName))
		end
	end
end
function en.write()
	enabledFlag = enabledFlag == "1" and "0" or "1"
	uci:set(packageName, "config", "enabled", enabledFlag)
	uci:save(packageName)
	uci:commit(packageName)
	if enabledFlag == "0" then
		luci.sys.init.stop(packageName)
	else
		luci.sys.init.enable(packageName)
		luci.sys.init.start(packageName)
	end
	luci.http.redirect(luci.dispatcher.build_url("admin/services/" .. packageName))
end

-- General options
s1 = c:section(NamedSection, "config", "vpn-policy-routing", translate("Configuration"))
s1.override_values = true
s1.override_depends = true
s1:tab("basic", translate("Basic Configuration"))

v = s1:taboption("basic", ListValue, "verbosity", translate("Output verbosity"),translate("Controls both system log and console output verbosity"))
v:value("0", translate("Suppress/No output"))
v:value("1", translate("Condensed output"))
v:value("2", translate("Verbose output"))
v.default = 2

se = s1:taboption("basic", ListValue, "strict_enforcement", translate("Strict enforcement"),translate("See the") .. " "
  .. [[<a href="]] .. readmeURL .. [[#strict-enforcement" target="_blank">]]
  .. translate("README") .. [[</a>]] .. " " .. translate("for details"))
se:value("0", translate("Do not enforce policies when their gateway is down"))
se:value("1", translate("Strictly enforce policies when their gateway is down"))
se.default = 1

dnsmasq = s1:taboption("basic", ListValue, "dnsmasq_enabled", translate("Use DNSMASQ for domain policies"),
	translate("Please check the" .. " "
  .. [[<a href="]] .. readmeURL .. [[#use-dnsmasq" target="_blank">]]
  .. translate("README") .. [[</a>]] .. " " .. translate("before enabling this option.")))
dnsmasq:value("0", translate("Disabled"))
dnsmasq:value("1", translate("Enabled"))

ipset = s1:taboption("basic", ListValue, "ipset_enabled", translate("Use ipsets"),
	translate("Please check the") .. " "
  .. [[<a href="]] .. readmeURL .. [[#additional-settings" target="_blank">]]
  .. translate("README") .. [[</a>]] .. " " .. translate("before changing this option."))
ipset:depends({dnsmasq_enabled="0"})
ipset:value("", translate("Disabled"))
ipset:value("1", translate("Enabled"))

ipv6 = s1:taboption("basic", ListValue, "ipv6_enabled", translate("IPv6 Support"))
ipv6:value("0", translate("Disabled"))
ipv6:value("1", translate("Enabled"))

s1:tab("advanced", translate("Advanced Configuration"),
	"<br/>&nbsp;&nbsp;&nbsp;&nbsp;<b>" .. translate("WARNING:") .. "</b>" .. " " .. translate("Please make sure to check the") .. " "
	.. [[<a href="]] .. readmeURL .. [[#additional-settings" target="_blank">]] .. translate("README") .. [[</a>]] .. " "
	.. translate("before changing anything in this section! Change any of the settings below with extreme caution!") .. "<br/><br/>")

supported = s1:taboption("advanced", DynamicList, "supported_interface", translate("Supported Interfaces"), translate("Allows to specify the list of interface names (in lower case) to be explicitly supported by the service. Can be useful if your OpenVPN tunnels have dev option other than tun* or tap*."))
supported.optional = false
supported.rmempty = true

ignored = s1:taboption("advanced", DynamicList, "ignored_interface", translate("Ignored Interfaces"), translate("Allows to specify the list of interface names (in lower case) to be ignored by the service. Can be useful if running both VPN server and VPN client on the router."))
ignored.optional = false
ignored.rmempty = true

iprule = s1:taboption("advanced", ListValue, "iprule_enabled", translate("IP Rules Support"), translate("Add an ip rule, not an iptables entry for policies with just the local address. Use with caution to manipulte policies priorities."))
iprule:value("", translate("Disabled"))
iprule:value("1", translate("Enabled"))
iprule.rmempty = true

udp = s1:taboption("advanced", ListValue, "udp_proto_enabled", translate("UDP Protocol Support"), translate("Add UDP protocol iptables rules for protocol policies with unset local addresses and either local or remote port set. By default (unless this is enabled) only TCP protocol iptables rules are added."))
udp:value("", translate("Disabled"))
udp:value("1", translate("Enabled"))
udp.rmempty = true

forward = s1:taboption("advanced", ListValue, "forward_chain_enabled", translate("Create FORWARD Chain"), translate("Create and use a FORWARD chain in the mangle table."))
forward:value("", translate("Disabled"))
forward:value("1", translate("Enabled"))
forward.rmempty = true

input = s1:taboption("advanced", ListValue, "input_chain_enabled", translate("Create INPUT Chain"), translate("Create and use an INPUT chain in the mangle table."))
input:value("", translate("Disabled"))
input:value("1", translate("Enabled"))
input.rmempty = true

output = s1:taboption("advanced", ListValue, "output_chain_enabled", translate("Create OUTPUT Chain"), translate("Create and use an OUTPUT chain in the mangle table. Policies in the OUTPUT chain will affect traffic from the router itself. All policies with unset local address will be duplicated in the OUTPUT chain."))
output:value("", translate("Disabled"))
output:value("1", translate("Enabled"))
output.rmempty = true

icmp = s1:taboption("advanced", ListValue, "icmp_interface", translate("Default ICMP Interface"), translate("Force the ICMP protocol interface."))
icmp:depends({output_chain_enabled="1"})
icmp:value("", translate("No Change"))
icmp:value("wan", translate("WAN"))
uci:foreach("network", "interface", function(s)
	local name=s['.name']
	if is_supported_interface(s) then icmp:value(name, string.upper(name)) end
end)
icmp.rmempty = true

wantid = s1:taboption("advanced", Value, "wan_tid", translate("WAN Table ID"), translate("Starting (WAN) Table ID number for tables created by the service."))
wantid.rmempty = true
wantid.placeholder = "201"

wantid = s1:taboption("advanced", Value, "wan_mark", translate("WAN Table FW Mark"), translate("Starting (WAN) FW Mark for marks used by the service. High starting mark is used to avoid conflict with SQM/QoS. Change with caution together with") .. " " .. translate("Service FW Mask") .. ".")
wantid.rmempty = true
wantid.placeholder = "0x010000"

wantid = s1:taboption("advanced", Value, "fw_mask", translate("Service FW Mask"), translate("FW Mask used by the service. High mask is used to avoid conflict with SQM/QoS. Change with caution together with") .. " " .. translate("WAN Table FW Mark") .. ".")
wantid.rmempty = true
wantid.placeholder = "0xff0000"

-- Policies
p = Map("vpn-policy-routing")
p.template="cbi/map"

s3 = p:section(TypedSection, "policy", translate("Policies"), translate("Comment, interface and at least one other field are required. Multiple local and remote addresses/devices/domains and ports can be space separated. Placeholders below represent just the format/syntax and will not be used if fields are left blank."))
s3.template = "cbi/tblsection"
s3.sortable  = true
s3.anonymous = true
s3.addremove = true

s3:option(Value, "comment", translate("Comment"))

la = s3:option(Value, "local_addresses", translate("Local addresses/devices"))
if laPlaceholder then
	la.placeholder = laPlaceholder
end
la.rmempty = true

lp = s3:option(Value, "local_ports", translate("Local ports"))
lp.datatype    = "list(neg(portrange))"
lp.placeholder = "0-65535"
lp.rmempty = true

ra = s3:option(Value, "remote_addresses", translate("Remote addresses/domains"))
ra.placeholder = "0.0.0.0/0"
ra.rmempty = true

rp = s3:option(Value, "remote_ports", translate("Remote ports"))
rp.datatype    = "list(neg(portrange))"
rp.placeholder = "0-65535"
rp.rmempty = true

gw = s3:option(ListValue, "interface", translate("Interface"))
-- gw.datatype = "network"
gw.rmempty = false
gw.default = "wan"
gw:value("wan","WAN")
uci:foreach("network", "interface", function(s)
	local name=s['.name']
	if is_supported_interface(s) then gw:value(name, string.upper(name)) end
end)

dscp = Map("vpn-policy-routing")
s6 = dscp:section(NamedSection, "config", "vpn-policy-routing", translate("DSCP Tagging"), translate("Set DSCP tags (in range between 1 and 63) for specific interfaces."))
wan = s6:option(Value, "wan_dscp", translate("WAN DSCP Tag"))
wan.datatype = "range(1,63)"
wan.rmempty = true
uci:foreach("network", "interface", function(s)
	local name=s['.name']
	if is_supported_interface(s) then s6:option(Value, name .. "_dscp", string.upper(name) .. " " .. translate("DSCP Tag")).rmempty = true end
end)

return c, p, dscp
