-- Copyright 2016 Eric Luehrsen <ericluehrsen@hotmail.com>
-- Licensed to the public under the Apache License 2.0.

local m3, s3, frm
local filename = "/etc/unbound/unbound_srv.conf"
local description =  translatef("Here you may edit the 'server:' clause in an internal 'include:'")
description = description .. " (" .. filename .. ")"

m3 = SimpleForm("editing", nil)
m3:append(Template("unbound/css-editing"))
m3.submit = translate("Save")
m3.reset = false
s3 = m3:section(SimpleSection, "Unbound Server Conf", description)
frm = s3:option(TextValue, "data")
frm.datatype = "string"
frm.rows = 20


function frm.cfgvalue()
  return nixio.fs.readfile(filename) or ""
end


function frm.write(self, section, data)
  return nixio.fs.writefile(filename, luci.util.trim(data:gsub("\r\n", "\n")))
end


return m3

