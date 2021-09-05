-- Copyright 2017 Yousong Zhou <yszhou4tech@gmail.com>
-- Licensed to the public under the Apache License 2.0.

local ss = require("luci.model.shadowsocks-libev")

local m, s, o

m = Map("shadowsocks-libev",
	translate("Redir Rules"),
	translate("On this page you can configure how traffics are to be \
		forwarded to ss-redir instances. \
		If enabled, packets will first have their src ip addresses checked \
		against <em>Src ip/net bypass</em>, <em>Src ip/net forward</em>, \
		<em>Src ip/net checkdst</em> and if none matches <em>Src default</em> \
		will give the default action to be taken. \
		If the prior check results in action <em>checkdst</em>, packets will continue \
		to have their dst addresses checked."))

local sdata = m:get('ss_rules')
if not sdata then
	m:set('ss_rules', nil, 'ss_rules')
	m:set('ss_rules', 'disabled', "1")
end

function src_dst_option(s, ...)
	local o = s:taboption(...)
	o.datatype = "or(ip4addr,cidr4)"
end

s = m:section(NamedSection, "ss_rules", "ss_rules")
s:tab("general", translate("General Settings"))
s:tab("src", translate("Source Settings"))
s:tab("dst", translate("Destination Settings"))

s:taboption('general', Flag, "disabled", translate("Disable"))
ss.option_install_package(s, 'general')

o = s:taboption('general', ListValue, "redir_tcp",
	translate("ss-redir for TCP"))
ss.values_redir(o, 'tcp')
o = s:taboption('general', ListValue, "redir_udp",
	translate("ss-redir for UDP"))
ss.values_redir(o, 'udp')

o = s:taboption('general', ListValue, "local_default",
	translate("Local-out default"),
	translate("Default action for locally generated TCP packets"))
ss.values_actions(o)
o = s:taboption('general', DynamicList, "ifnames",
	translate("Ingress interfaces"),
	translate("Only apply rules on packets from these network interfaces"))
ss.values_ifnames(o)
s:taboption('general', Value, "ipt_args",
	translate("Extra arguments"),
	translate("Passes additional arguments to iptables. Use with care!"))

src_dst_option(s, 'src', DynamicList, "src_ips_bypass",
	translate("Src ip/net bypass"),
	translate("Bypass ss-redir for packets with src address in this list"))
src_dst_option(s, 'src', DynamicList, "src_ips_forward",
	translate("Src ip/net forward"),
	translate("Forward through ss-redir for packets with src address in this list"))
src_dst_option(s, 'src', DynamicList, "src_ips_checkdst",
	translate("Src ip/net checkdst"),
	translate("Continue to have dst address checked for packets with src address in this list"))
o = s:taboption('src', ListValue, "src_default",
	translate("Src default"),
	translate("Default action for packets whose src address do not match any of the src ip/net list"))
ss.values_actions(o)

src_dst_option(s, 'dst', DynamicList, "dst_ips_bypass",
	translate("Dst ip/net bypass"),
	translate("Bypass ss-redir for packets with dst address in this list"))
src_dst_option(s, 'dst', DynamicList, "dst_ips_forward",
	translate("Dst ip/net forward"),
	translate("Forward through ss-redir for packets with dst address in this list"))

o = s:taboption('dst', FileBrowser, "dst_ips_bypass_file",
	translate("Dst ip/net bypass file"),
	translate("File containing ip/net for the purposes as with <em>Dst ip/net bypass</em>"))
o.datatype = "file"
s:taboption('dst', FileBrowser, "dst_ips_forward_file",
	translate("Dst ip/net forward file"),
	translate("File containing ip/net for the purposes as with <em>Dst ip/net forward</em>"))
o.datatype = "file"
o = s:taboption('dst', ListValue, "dst_default",
	translate("Dst default"),
	translate("Default action for packets whose dst address do not match any of the dst ip list"))
ss.values_actions(o)

local installed = os.execute("iptables -m recent -h &>/dev/null") == 0
if installed then
	o = s:taboption('dst', Flag, "dst_forward_recentrst")
else
	m:set('ss_rules', 'dst_forward_recentrst', "0")
	o = s:taboption("dst", Button, "_install")
	o.inputtitle = translate("Install package iptables-mod-conntrack-extra")
	o.inputstyle = "apply"
	o.write = function()
		return luci.http.redirect(
			luci.dispatcher.build_url("admin/system/packages") ..
			"?submit=1&install=iptables-mod-conntrack-extra"
		)
	end
end
o.title = translate("Forward recentrst")
o.description = translate("Forward those packets whose dst have recently sent to us multiple tcp-rst")

return m
