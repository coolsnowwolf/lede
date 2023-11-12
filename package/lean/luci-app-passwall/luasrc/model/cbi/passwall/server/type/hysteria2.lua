local m, s = ...

local api = require "luci.passwall.api"

if not api.finded_com("hysteria") then
	return
end

local type_name = "Hysteria2"

local option_prefix = "hysteria2_"

local function option_name(name)
	return option_prefix .. name
end

-- [[ Hysteria2 ]]

s.fields["type"]:value(type_name, "Hysteria2")

o = s:option(Value, option_name("port"), translate("Listen Port"))
o.datatype = "port"

o = s:option(Value, option_name("obfs"), translate("Obfs Password"))
o.rewrite_option = o.option

o = s:option(Value, option_name("auth_password"), translate("Auth Password"))
o.password = true
o.rewrite_option = o.option

o = s:option(Flag, option_name("udp"), translate("UDP"))
o.default = "1"
o.rewrite_option = o.option

o = s:option(Value, option_name("up_mbps"), translate("Max upload Mbps"))
o.rewrite_option = o.option

o = s:option(Value, option_name("down_mbps"), translate("Max download Mbps"))
o.rewrite_option = o.option

o = s:option(Flag, option_name("ignoreClientBandwidth"), translate("ignoreClientBandwidth"))
o.default = "0"
o.rewrite_option = o.option

o = s:option(FileUpload, option_name("tls_certificateFile"), translate("Public key absolute path"), translate("as:") .. "/etc/ssl/fullchain.pem")
o.default = m:get(s.section, "tls_certificateFile") or "/etc/config/ssl/" .. arg[1] .. ".pem"
o.validate = function(self, value, t)
	if value and value ~= "" then
		if not nixio.fs.access(value) then
			return nil, translate("Can't find this file!")
		else
			return value
		end
	end
	return nil
end

o = s:option(FileUpload, option_name("tls_keyFile"), translate("Private key absolute path"), translate("as:") .. "/etc/ssl/private.key")
o.default = m:get(s.section, "tls_keyFile") or "/etc/config/ssl/" .. arg[1] .. ".key"
o.validate = function(self, value, t)
	if value and value ~= "" then
		if not nixio.fs.access(value) then
			return nil, translate("Can't find this file!")
		else
			return value
		end
	end
	return nil
end

o = s:option(Flag, option_name("log"), translate("Log"))
o.default = "1"
o.rmempty = false

api.luci_types(arg[1], m, s, type_name, option_prefix)
