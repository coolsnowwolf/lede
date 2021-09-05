-- Copyright 2017-2018 Dirk Brenken (dev@brenken.org)
-- This is free software, licensed under the Apache License, Version 2.0

local fs        = require("nixio.fs")
local uci       = require("luci.model.uci").cursor()
local util      = require("luci.util")
local res_input = "/usr/share/dnscrypt-proxy/dnscrypt-resolvers.csv"
local res_dir   = fs.dirname(res_input)
local dump      = util.ubus("network.interface", "dump", {})
local plug_cnt  = tonumber(luci.sys.exec("env -i /usr/sbin/dnscrypt-proxy --version | grep 'Support for plugins: present' | wc -l"))
local res_list  = {}
local url       = "https://raw.githubusercontent.com/dyne/dnscrypt-proxy/master/dnscrypt-resolvers.csv"

local _, date = pcall(require, "luci.http.date")
if not date then
	_, date = pcall(require, "luci.http.protocol.date")
end

if not fs.access(res_input) then
	if not fs.access("/lib/libustream-ssl.so") then
		m = SimpleForm("error", nil, translate("No default resolver list and no SSL support available.<br />")
			.. translate("Please install a resolver list to '/usr/share/dnscrypt-proxy/dnscrypt-resolvers.csv' to use this package."))
		m.submit = false
		m.reset = false
		return m
	else
		luci.sys.call("env -i /bin/uclient-fetch --no-check-certificate -O " .. res_input .. " " .. url .. " >/dev/null 2>&1")
	end
end

if not uci:get_first("dnscrypt-proxy", "global") then
	uci:add("dnscrypt-proxy", "global")
	uci:save("dnscrypt-proxy")
	uci:commit("dnscrypt-proxy")
end

if fs.access(res_input) then
	for line in io.lines(res_input) or {} do
		local name,
		location,
		dnssec,
		nolog = line:match("^([^,]+),.-,\".-\",\"*(.-)\"*,.-,[0-9],\"*([yesno]+)\"*,\"*([yesno]+)\"*,.*")
		if name ~= "" and name ~= "Name" then
			if location == "" then
				location = "-"
			end
			if dnssec == "" then
				dnssec = "-"
			end
			if nolog == "" then
				nolog = "-"
			end
			res_list[#res_list + 1] = { name = name, location = location, dnssec = dnssec, nolog = nolog }
		end
	end
end

m = Map("dnscrypt-proxy", translate("DNSCrypt-Proxy"),
	translate("Configuration of the DNSCrypt-Proxy package. ")
	.. translatef("For further information "
	.. "<a href=\"%s\" target=\"_blank\">"
	.. "see the wiki online</a>", "https://wiki.openwrt.org/inbox/dnscrypt"))
m:chain("dhcp")

function m.on_after_commit(self)
	function d1.validate(self, value, s1)
		if value == "1" then
			uci:commit("dnscrypt-proxy")
			uci:set("dhcp", s1, "noresolv", 1)
			if not fs.access("/etc/resolv-crypt.conf") or fs.stat("/etc/resolv-crypt.conf").size == 0 then
				uci:set("dhcp", s1, "resolvfile", "/tmp/resolv.conf.auto")
			else
				uci:set("dhcp", s1, "resolvfile", "/etc/resolv-crypt.conf")
			end
			local server_list = {}
			local cnt = 1
			uci:foreach("dnscrypt-proxy", "dnscrypt-proxy", function(s)
				server_list[cnt] = s['address'] .. "#" .. s['port']
				cnt = cnt + 1
			end)
			server_list[cnt] = "/pool.ntp.org/8.8.8.8"
			uci:set_list("dhcp", s1, "server", server_list)
			if cnt > 2 then
				uci:set("dhcp", s1, "allservers", 1)
			else
				uci:set("dhcp", s1, "allservers", 0)
			end
			uci:save("dhcp")
			uci:commit("dhcp")
		end
		return value
	end
	luci.sys.call("env -i /etc/init.d/dnscrypt-proxy restart >/dev/null 2>&1")
	luci.sys.call("env -i /etc/init.d/dnsmasq restart >/dev/null 2>&1")
end

s = m:section(TypedSection, "global", translate("General Options"))
s.anonymous = true

-- Main dnscrypt-proxy resource list

o1 = s:option(DummyValue, "", translate("Default Resolver List"))
o1.template = "dnscrypt-proxy/res_options"
o1.value = res_input

o2 = s:option(DummyValue, "", translate("File Date"))
o2.template = "dnscrypt-proxy/res_options"
if fs.access(res_input) then
	o2.value = date.to_http(fs.stat(res_input).mtime)
else
	o2.value = "-"
end

o3 = s:option(DummyValue, "", translate("File Checksum"))
o3.template = "dnscrypt-proxy/res_options"
if fs.access(res_input) then
	o3.value = luci.sys.exec("sha256sum " .. res_input .. " | awk '{print $1}'")
else
	o3.value = "-"
end

if fs.access("/lib/libustream-ssl.so") then
	btn1 = s:option(Button, "", translate("Refresh Resolver List"),
		translate("Download the current resolver list from 'github.com/dyne/dnscrypt-proxy'."))
	btn1.inputtitle = translate("Refresh List")
	btn1.inputstyle = "apply"
	btn1.disabled = false
	function btn1.write()
		if not fs.access(res_dir) then
			fs.mkdir(res_dir)
		end
		luci.sys.call("env -i /bin/uclient-fetch --no-check-certificate -O " .. res_input .. " " .. url .. " >/dev/null 2>&1")
		luci.http.redirect(luci.dispatcher.build_url("admin", "services", "dnscrypt-proxy"))
	end
else
	btn1 = s:option(Button, "", translate("Refresh Resolver List"),
		translate("No SSL support available.<br />")
		.. translate("Please install a 'libustream-ssl' library to download the current resolver list from 'github.com/dyne/dnscrypt-proxy'."))
	btn1.inputtitle = translate("-------")
	btn1.inputstyle = "button"
	btn1.disabled = true
end

if not fs.access("/etc/resolv-crypt.conf") or fs.stat("/etc/resolv-crypt.conf").size == 0 then
	btn2 = s:option(Button, "", translate("Create Custom Config File"),
		translate("Create '/etc/resolv-crypt.conf' with 'options timeout:1' to reduce DNS upstream timeouts with multiple DNSCrypt instances.<br />")
		.. translatef("For further information "
		.. "<a href=\"%s\" target=\"_blank\">"
		.. "see the wiki online</a>", "https://wiki.openwrt.org/inbox/dnscrypt"))
	btn2.inputtitle = translate("Create Config File")
	btn2.inputstyle = "apply"
	btn2.disabled = false
	function btn2.write()
		luci.sys.call("env -i echo 'options timeout:1' > '/etc/resolv-crypt.conf'")
		luci.http.redirect(luci.dispatcher.build_url("admin", "services", "dnscrypt-proxy"))
	end
else
	btn2 = s:option(Button, "", translate("Create Custom Config File"),
		translate("The config file '/etc/resolv-crypt.conf' already exist.<br />")
		.. translate("Please edit the file manually in the 'Advanced' section."))
	btn2.inputtitle = translate("-------")
	btn2.inputstyle = "button"
	btn2.disabled = true
end

-- Trigger settings

t = s:option(ListValue, "procd_trigger", translate("Startup Trigger"),
	translate("By default the DNSCrypt-Proxy startup will be triggered by ifup events of 'All' available network interfaces.<br />")
	.. translate("To restrict the trigger, select only the relevant network interface. Usually the 'wan' interface should work for most users."))
t:value("", "All")
if dump then
	local i, v
	for i, v in ipairs(dump.interface) do
		if v.interface ~= "loopback" then
			t:value(v.interface)
		end
	end
end
t.default = procd_trigger or "All"
t.rmempty = true

-- Mandatory options per instance

s = m:section(TypedSection, "dnscrypt-proxy", translate("Instance Options"))
s.anonymous = true
s.addremove = true

i1 = s:option(Value, "address", translate("IP Address"),
	translate("The local IPv4 or IPv6 address. The latter one should be specified within brackets, e.g. '[::1]'."))
i1.default = address or "127.0.0.1"
i1.rmempty = false

i2 = s:option(Value, "port", translate("Port"),
	translate("The listening port for DNS queries."))
i2.datatype = "port"
i2.default = port
i2.rmempty = false

i3 = s:option(ListValue, "resolver", translate("Resolver (LOC/SEC/NOLOG)"),
	translate("Name of the remote DNS service for resolving queries incl. Location, DNSSEC- and NOLOG-Flag."))
i3.datatype = "hostname"
i3.widget = "select"
local i, v
for i, v in ipairs(res_list) do
	if v.name then
		i3:value(v.name, v.name .. " (" .. v.location .. "/" .. v.dnssec .. "/" .. v.nolog .. ")")
	end
end
i3.default = resolver
i3.rmempty = false

-- Extra options per instance

e1 = s:option(Value, "resolvers_list", translate("Alternate Resolver List"),
	translate("Specify a non-default Resolver List."))
e1.datatype = "file"
e1.optional = true

e2 = s:option(Value, "ephemeral_keys", translate("Ephemeral Keys"),
	translate("Improve privacy by using an ephemeral public key for each query. ")
	.. translate("This option requires extra CPU cycles and is useless with most DNSCrypt server."))
e2.datatype = "bool"
e2.value = 1
e2.optional = true

if plug_cnt > 0 then
	e3 = s:option(DynamicList, "blacklist", translate("Blacklist"),
		translate("Local blacklists allow you to block abuse sites by domains or ip addresses. ")
		.. translate("The value for this property is the blocklist type and path to the file, e.g.'domains:/path/to/dbl.txt' or 'ips:/path/to/ipbl.txt'."))
	e3.optional = true

	e4 = s:option(Value, "block_ipv6", translate("Block IPv6"),
		translate("Disable IPv6 to speed up DNSCrypt-Proxy."))
	e4.datatype = "bool"
	e4.value = 1
	e4.optional = true

	e5 = s:option(Value, "local_cache", translate("Local Cache"),
		translate("Enable Caching to speed up DNSCcrypt-Proxy."))
	e5.datatype = "bool"
	e5.value = 1
	e5.optional = true
	
	e6 = s:option(Value, "query_log_file", translate("DNS Query Logfile"),
	translate("Log the received DNS queries to a file, so you can watch in real-time what is happening on the network."))
	e6.optional = true
end

-- Dnsmasq options

m1 = Map("dhcp")

s1 = m1:section(TypedSection, "dnsmasq", translate("Dnsmasq Options"))
s1.anonymous = true

d1 = s1:option(Flag, "", translate("Transfer Options To Dnsmasq"),
	translate("Apply DNSCrypt-Proxy specific settings to the Dnsmasq configuration.<br />")
	.. translate("Please note: This may change the values for 'noresolv', 'resolvfile', 'allservers' and the list 'server' settings."))
d1.default = d1.enabled
d1.rmempty = false

return m, m1
