local m, s = ...

local api = require "luci.passwall.api"

if not api.finded_com("trojan-go") then
	return
end

local type_name = "Trojan-Go"

local option_prefix = "trojan_go_"

local function option_name(name)
	return option_prefix .. name
end

local encrypt_methods_ss_aead = {
	"chacha20-ietf-poly1305",
	"aes-128-gcm",
	"aes-256-gcm",
}

-- [[ Trojan Go ]]

s.fields["type"]:value(type_name, "Trojan-Go")

o = s:option(Value, option_name("address"), translate("Address (Support Domain Name)"))

o = s:option(Value, option_name("port"), translate("Port"))
o.datatype = "port"

o = s:option(Value, option_name("password"), translate("Password"))
o.password = true

o = s:option(ListValue, option_name("tcp_fast_open"), "TCP " .. translate("Fast Open"), translate("Need node support required"))
o:value("false")
o:value("true")

o = s:option(Flag, option_name("tls"), translate("TLS"))
o.default = 1

o = s:option(Flag, option_name("tls_allowInsecure"), translate("allowInsecure"), translate("Whether unsafe connections are allowed. When checked, Certificate validation will be skipped."))
o.default = "0"
o:depends({ [option_name("tls")] = true })

o = s:option(Value, option_name("tls_serverName"), translate("Domain"))
o:depends({ [option_name("tls")] = true })

o = s:option(Flag, option_name("tls_sessionTicket"), translate("Session Ticket"))
o.default = "0"
o:depends({ [option_name("tls")] = true })

o = s:option(ListValue, option_name("fingerprint"), translate("Finger Print"))
o:value("disable", translate("Disable"))
o:value("firefox")
o:value("chrome")
o:value("ios")
o.default = "disable"
o:depends({ [option_name("tls")] = true })

o = s:option(ListValue, option_name("transport"), translate("Transport"))
o:value("original", translate("Original"))
o:value("ws", "WebSocket")
o.default = "original"
o.rewrite_option = "trojan_transport"

o = s:option(ListValue, option_name("plugin_type"), translate("Transport Plugin"))
o:value("plaintext", "Plain Text")
o:value("shadowsocks", "ShadowSocks")
o:value("other", "Other")
o.default = "plaintext"
o:depends({ [option_name("tls")] = false, [option_name("transport")] = "original" })

o = s:option(Value, option_name("plugin_cmd"), translate("Plugin Binary"))
o.placeholder = "eg: /usr/bin/v2ray-plugin"
o:depends({ [option_name("plugin_type")] = "shadowsocks" })
o:depends({ [option_name("plugin_type")] = "other" })

o = s:option(Value, option_name("plugin_option"), translate("Plugin Option"))
o.placeholder = "eg: obfs=http;obfs-host=www.baidu.com"
o:depends({ [option_name("plugin_type")] = "shadowsocks" })
o:depends({ [option_name("plugin_type")] = "other" })

o = s:option(DynamicList, option_name("plugin_arg"), translate("Plugin Option Args"))
o.placeholder = "eg: [\"-config\", \"test.json\"]"
o:depends({ [option_name("plugin_type")] = "shadowsocks" })
o:depends({ [option_name("plugin_type")] = "other" })

o = s:option(Value, option_name("ws_host"), translate("WebSocket Host"))
o:depends({ [option_name("transport")] = "ws" })

o = s:option(Value, option_name("ws_path"), translate("WebSocket Path"))
o.placeholder = "/"
o:depends({ [option_name("transport")] = "ws" })

-- [[ Shadowsocks2 ]] --
o = s:option(Flag, option_name("ss_aead"), translate("Shadowsocks secondary encryption"))
o.default = "0"

o = s:option(ListValue, option_name("ss_aead_method"), translate("Encrypt Method"))
for _, v in ipairs(encrypt_methods_ss_aead) do o:value(v, v) end
o.default = "aes-128-gcm"
o:depends({ [option_name("ss_aead")] = true })

o = s:option(Value, option_name("ss_aead_pwd"), translate("Password"))
o.password = true
o:depends({ [option_name("ss_aead")] = true })

o = s:option(Flag, option_name("smux"), translate("Smux"))

o = s:option(Value, option_name("mux_concurrency"), translate("Mux concurrency"))
o.default = 8
o:depends({ [option_name("smux")] = true })

o = s:option(Value, option_name("smux_idle_timeout"), translate("Mux idle timeout"))
o.default = 60
o:depends({ [option_name("smux")] = true })

api.luci_types(arg[1], m, s, type_name, option_prefix)
