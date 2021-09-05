local m,s,o

m=Map("bypass")
s=m:section(TypedSection,"server_global",translate("Global Setting"))
s.anonymous=true

o=s:option(Flag,"enable_server",translate("Enable Server"))

s=m:section(TypedSection,"server_config",translate("Server Setting"))
s.anonymous=true
s.addremove=true
s.template="cbi/tblsection"
s.extedit=luci.dispatcher.build_url("admin/services/bypass/server/%s")
function s.create(...)
	local sid=TypedSection.create(...)
	if sid then
		luci.http.redirect(s.extedit%sid)
		return
	end
end

o=s:option(Flag,"enable",translate("Enable"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("0")
end
o.rmempty=false

o=s:option(DummyValue,"type",translate("Server Type"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or "-"
end

o=s:option(DummyValue,"server_port",translate("Server Port"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or "-"
end

o=s:option(DummyValue,"username",translate("Username"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or "-"
end

o=s:option(DummyValue,"encrypt_method_ss",translate("Encrypt Method (SS)"))
function o.cfgvalue(...)
	local v=Value.cfgvalue(...)
	return v and v:upper() or "-"
end

o=s:option(DummyValue,"plugin",translate("Plugin (SS)"))
function o.cfgvalue(...)
	local v=Value.cfgvalue(...)
	return v and v:upper() or "-"
end

o=s:option(DummyValue,"encrypt_method",translate("Encrypt Method"))
function o.cfgvalue(...)
	local v=Value.cfgvalue(...)
	return v and v:upper() or "-"
end

o=s:option(DummyValue,"protocol",translate("Protocol"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or "-"
end

o=s:option(DummyValue,"obfs",translate("Obfs"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or "-"
end

return m
