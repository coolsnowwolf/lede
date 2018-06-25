local m, s, o

m = Map("udp2raw", "%s - %s" %{translate("udp2raw-tunnel"), translate("Servers Manage")})

s = m:section(TypedSection, "servers")
s.anonymous = true
s.addremove = true
s.sortable = true
s.template = "cbi/tblsection"
s.extedit = luci.dispatcher.build_url("admin/services/udp2raw/servers/%s")
function s.create(...)
	local sid = TypedSection.create(...)
	if sid then
		luci.http.redirect(s.extedit % sid)
		return
	end
end

o = s:option(DummyValue, "alias", translate("Alias"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end

o = s:option(DummyValue, "_server_address", translate("Server Address"))
function o.cfgvalue(self, section)
	local server_addr = m.uci:get("udp2raw", section, "server_addr") or "?"
	local server_port = m.uci:get("udp2raw", section, "server_port") or "8080"
	return "%s:%s" %{server_addr, server_port}
end

o = s:option(DummyValue, "_listen_address", translate("Listen Address"))
function o.cfgvalue(self, section)
	local listen_addr = m.uci:get("udp2raw", section, "listen_addr") or "127.0.0.1"
	local listen_port = m.uci:get("udp2raw", section, "listen_port") or "2080"
	return "%s:%s" %{listen_addr, listen_port}
end

o = s:option(DummyValue, "raw_mode", translate("Raw Mode"))
function o.cfgvalue(...)
	local v = Value.cfgvalue(...)
	return v and v:lower() or "faketcp"
end

o = s:option(DummyValue, "cipher_mode", translate("Cipher Mode"))
function o.cfgvalue(...)
	local v = Value.cfgvalue(...)
	return v and v:lower() or "aes128cbc"
end

o = s:option(DummyValue, "auth_mode", translate("Auth Mode"))
function o.cfgvalue(...)
	local v = Value.cfgvalue(...)
	return v and v:lower() or "md5"
end

return m
