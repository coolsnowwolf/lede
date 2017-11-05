-- Copyright (C) 2014-2017 Jian Chang <aa65535@live.com>
-- Copyright (C) 2017 Ian Li <OpenSource@ianli.xyz>
-- Licensed to the public under the GNU General Public License v3.

local m, s, o
local transparent_proxy = "transparent-proxy"
local uci = luci.model.uci.cursor()

local function has_udp_relay()
	return luci.sys.call("lsmod | grep -q TPROXY && command -v ip >/dev/null") == 0
end

local has_tproxy = has_udp_relay()

m = Map(transparent_proxy, "%s - %s" %{translate("Transparent Proxy"), translate("General Settings")})
m.template = "transparent-proxy/general"

-- [[ Running Status ]]--
s = m:section(TypedSection, "general", translate("Working Status"))
s.anonymous = true

o = s:option(DummyValue, "_status_rules", translate("Proxy Rules"), translate("The connections will not be redirected unless the Proxy Rules exist."))
o.value = "<span id=\"_status_rules\">%s</span>" %{translate("Collecting data...")}
o.rawhtml = true

o = s:option(DummyValue, "_status_ss_redir", translate("Shadowsocks"))
o.value = "<span id=\"_status_ss_redir\">%s</span>" %{translate("Collecting data...")}
o.rawhtml = true

o = s:option(DummyValue, "_status_ssr_redir", translate("ShadowsocksR"))
o.value = "<span id=\"_status_ssr_redir\">%s</span>" %{translate("Collecting data...")}
o.rawhtml = true

o = s:option(DummyValue, "_status_v2ray", translate("V2Ray"))
o.value = "<span id=\"_status_v2ray\">%s</span>" %{translate("Collecting data...")}
o.rawhtml = true

o = s:option(DummyValue, "_status_redsocks", translate("Redsocks"))
o.value = "<span id=\"_status_redsocks\">%s</span>" %{translate("Collecting data...")}
o.rawhtml = true

s = m:section(TypedSection, "general", translate("Global Settings"))
s.anonymous = true

o = s:option(Flag, "enable", translate("Enable"))
o.default = "0"
o.rmempty = false

if has_tproxy then
	o = s:option(Flag, "udp", translate("UDP Relay"))
	o.default = "0"
	o.rmempty = false
end

o = s:option(Value, "startup_delay", translate("Startup Delay"))
o:value(0, translate("Not enabled"))
for _, v in ipairs({5, 10, 15, 25, 40}) do
	o:value(v, translate("%u seconds") %{v})
end
o.datatype = "uinteger"
o.default = 0
o.rmempty = false

o = s:option(Value, "redirect_port", translate("Redirect Port"), translate("The matched connections will be redirected to this port. There should be a port forwarding service listening on this port, such as RedSock, V2Ray (dokodemo-door), Shadowsocks (ss-redir) and so on."))
o.placeholder = 1234
o.default = 1234
o.datatype = "port"
o.rmempty = false

return m
