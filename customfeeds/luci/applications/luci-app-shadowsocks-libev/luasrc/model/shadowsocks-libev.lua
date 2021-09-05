-- Copyright 2017 Yousong Zhou <yszhou4tech@gmail.com>
-- Licensed to the public under the Apache License 2.0.

local _up = getfenv(3)
local ut = require("luci.util")
local sys = require("luci.sys")
local ds = require("luci.dispatcher")
local nw = require("luci.model.network")
nw.init()
module("luci.model.shadowsocks-libev", function(m)
	setmetatable(m, {__index=function (self, k)
		local tb = _up
		return rawget(self, k) or _up[k]
	end})
end)

function values_actions(o)
	o:value("bypass")
	o:value("forward")
	if o.option ~= "dst_default" then
		o:value("checkdst")
	end
end

function values_redir(o, xmode)
	o.map.uci.foreach("shadowsocks-libev", "ss_redir", function(sdata)
		local disabled = ucival_to_bool(sdata["disabled"])
		local sname = sdata[".name"]
		local mode = sdata["mode"] or "tcp_only"
		if not disabled and mode:find(xmode) then
			local desc = "%s - %s" % {sname, mode}
			o:value(sname, desc)
		end
	end)
	o:value("", "<unset>")
	o.default = ""
end

function values_serverlist(o)
	o.map.uci.foreach("shadowsocks-libev", "server", function(sdata)
		local sname = sdata[".name"]
		local server = sdata["server"]
		local server_port = sdata["server_port"]
		if server and server_port then
			local disabled = ucival_to_bool(sdata[".disabled"]) and " - disabled" or ""
			local desc = "%s - %s:%s%s" % {sname, server, server_port, disabled}
			o:value(sname, desc)
		end
	end)
end

function values_ipaddr(o)
	for _, v in ipairs(nw:get_interfaces()) do
		for _, a in ipairs(v:ipaddrs()) do
			o:value(a:host():string(), '%s (%s)' %{ a:host(), v:shortname() })
		end
	end
end

function values_ifnames(o)
	for _, v in ipairs(sys.net.devices()) do
		o:value(v)
	end
end

function options_client(s, tab)
	local o

	o = s:taboption(tab, ListValue, "server", translate("Remote server"))
	values_serverlist(o)
	o = s:taboption(tab, Value, "local_address", translate("Local address"))
	o.datatype = "ipaddr"
	o.placeholder = "0.0.0.0"
	values_ipaddr(o)
	o = s:taboption(tab, Value, "local_port", translate("Local port"))
	o.datatype = "port"
end

function options_server(s, tab)
	local o
	local optfunc

	if tab == nil then
		optfunc = function(...) return s:option(...) end
	else
		optfunc = function(...) return s:taboption(tab, ...) end
	end

	o = optfunc(Value, "server", translate("Server"))
	o.datatype = "host"
	o.size = 16
	o = optfunc(Value, "server_port", translate("Server port"))
	o.datatype = "port"
	o.size = 5
	o = optfunc(ListValue, "method", translate("Method"))
	for _, m in ipairs(methods) do
		o:value(m)
	end
	o = optfunc(Value, "key", translate("Key (base64 encoding)"))
	o.datatype = "base64"
	o.password = true
	o.size = 12
	o = optfunc(Value, "password", translate("Password"))
	o.password = true
	o.size = 12
end

function options_common(s, tab)
	local o

	o = s:taboption(tab, ListValue, "mode", translate("Mode of operation"))
	for _, m in ipairs(modes) do
		o:value(m)
	end
	o.default = "tcp_and_udp"
	o = s:taboption(tab, Value, "mtu", translate("MTU"))
	o.datatype = "uinteger"
	o = s:taboption(tab, Value, "timeout", translate("Timeout (sec)"))
	o.datatype = "uinteger"
	s:taboption(tab, Value, "user", translate("Run as"))

	s:taboption(tab, Flag, "verbose", translate("Verbose"))
	s:taboption(tab, Flag, "ipv6_first", translate("IPv6 First"), translate("Prefer IPv6 addresses when resolving names"))
	s:taboption(tab, Flag, "fast_open", translate("Enable TCP Fast Open"))
	s:taboption(tab, Flag, "no_delay", translate("Enable TCP_NODELAY"))
	s:taboption(tab, Flag, "reuse_port", translate("Enable SO_REUSEPORT"))
end

function ucival_to_bool(val)
	return val == "true" or val == "1" or val == "yes" or val == "on"
end

function cfgvalue_overview(sdata)
	local stype = sdata[".type"]
	local lines  = {}

	if stype == "ss_server" then
		cfgvalue_overview_(sdata, lines, names_options_server)
		cfgvalue_overview_(sdata, lines, names_options_common)
		cfgvalue_overview_(sdata, lines, {
			"bind_address",
		})
	elseif stype == "ss_local" or stype == "ss_redir" or stype == "ss_tunnel" then
		cfgvalue_overview_(sdata, lines, names_options_client)
		if stype == "ss_tunnel" then
			cfgvalue_overview_(sdata, lines, {"tunnel_address"})
		end
		cfgvalue_overview_(sdata, lines, names_options_common)
	else
		return nil, nil
	end
	local sname = sdata[".name"]
	local key = "%s.%s" % {stype, sname}
	local value = {
		[".name"] = sname,
		name = '%s.<var>%s</var>' % {stype, sname},
		overview = table.concat(lines, "</br>"),
		disabled = ucival_to_bool(sdata["disabled"]),
	}
	return key, value
end

function cfgvalue_overview_(sdata, lines, names)
	local line

	for _, n in ipairs(names) do
		local v = sdata[n]
		if v ~= nil then
			if n == "key" or n == "password" then
				v = translate("<hidden>")
			end
			local fv = "<var>%s</var>" % ut.pcdata(v)
			if sdata[".type"] ~= "ss_server" and n == "server" then
				fv = '<a class="label" href="%s">%s</a>' % {
					ds.build_url("admin/services/shadowsocks-libev/servers", v), fv}
			end
			line = n .. ": " .. fv
			table.insert(lines, line)
		end
	end
end

function option_install_package(s, tab)
	local bin = s.sectiontype:gsub("_", "-", 1)
	local installed = nixio.fs.access("/usr/bin/" .. bin)
	if installed then
		return
	end
	local opkg_package = "shadowsocks-libev-" .. bin
	local p_install
	if tab then
		p_install = s:taboption(tab, Button, "_install")
	else
		p_install = s:option(Button, "_install")
	end
	p_install.title      = translate("Package is not installed")
	p_install.inputtitle = translate("Install package %q" % opkg_package)
	p_install.inputstyle = "apply"

	function p_install.write()
		return luci.http.redirect(
			luci.dispatcher.build_url("admin/system/packages") ..
			"?submit=1&install=%s" % opkg_package
		)
	end
end

names_options_server = {
	"server",
	"server_port",
	"method",
	"key",
	"password",
}

names_options_client = {
	"server",
	"local_address",
	"local_port",
}

names_options_common = {
	"verbose",
	"ipv6_first",
	"fast_open",
	"no_delay",
	"reuse_port",
	"mode",
	"mtu",
	"timeout",
	"user",
}

modes = {
	"tcp_only",
	"tcp_and_udp",
	"udp_only",
}

methods = {
	-- aead
	"aes-128-gcm",
	"aes-192-gcm",
	"aes-256-gcm",
	"chacha20-ietf-poly1305",
	"xchacha20-ietf-poly1305",
	-- stream
	"table",
	"rc4",
	"rc4-md5",
	"aes-128-cfb",
	"aes-192-cfb",
	"aes-256-cfb",
	"aes-128-ctr",
	"aes-192-ctr",
	"aes-256-ctr",
	"bf-cfb",
	"camellia-128-cfb",
	"camellia-192-cfb",
	"camellia-256-cfb",
	"salsa20",
	"chacha20",
	"chacha20-ietf",
}
