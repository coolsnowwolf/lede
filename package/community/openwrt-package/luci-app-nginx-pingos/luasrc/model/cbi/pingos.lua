m = Map("pingos", translate("PingOS"))
m:append(Template("pingos/status"))

s = m:section(TypedSection, "global")
s.anonymous = true
s.addremove = false

s:tab("global",  translate("Global Settings"))
s:tab("template", translate("Edit Template"))

nginx = s:taboption("template", Value, "_nginx", translatef("Edit the template that is used for generating the %s configuration.", "nginx"),
	translatef("This is the content of the file '%s'", "/etc/pingos.template") .. "<br />" ..
	translatef("Values enclosed by pipe symbols ('|') should not be changed. They get their values from the '%s' tab.", translate("Global Settings")))
nginx.template = "cbi/tvalue"
nginx.rows = 30

function nginx.cfgvalue(self, section)
	return nixio.fs.readfile("/etc/pingos.template")
end

function nginx.write(self, section, value)
	value = value:gsub("\r\n?", "\n")
	nixio.fs.writefile("/etc/pingos.template", value)
end

o = s:taboption("global", Flag, "enable", translate("Enable"))
o.rmempty = false

o = s:taboption("global", Flag, "ipv6", translate("Listen IPv6"))
o.rmempty = false

o = s:taboption("global", Value, "http_port", "HTTP(S)" ..translate("Port"))
o.datatype = "port"
o.default = 8082
o.rmempty = false

o = s:taboption("global", Flag, "https", translate("HTTPS"))
o.rmempty = false

o = s:taboption("global", FileUpload, "certificate", translate("certificate"))
o:depends("https", 1)

o = s:taboption("global", FileUpload, "key", translate("key"))
o:depends("https", 1)

o = s:taboption("global", Value, "rtmp_port", "RTMP" ..translate("Port"))
o.datatype = "port"
o.default = 1935
o.rmempty = false

o = s:taboption("global", Flag, "hls", translate("HLS"))
o.rmempty = false

o = s:taboption("global", Flag, "hls2", translate("HLS2"))
o.rmempty = false

o = s:taboption("global", Flag, "ts_record", "TS " .. translate("Record"))
o.rmempty = false

o = s:taboption("global", Flag, "flv_record", "FLV " .. translate("Record"))
o.rmempty = false

o = s:taboption("global", Value, "record_path", translate("Record") .. translate("Path"))
o.default = "/tmp/record"
o:depends("ts_record", 1)
o:depends("flv_record", 1)

return m
