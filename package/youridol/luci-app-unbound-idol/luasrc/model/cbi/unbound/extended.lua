-- Copyright 2016 Eric Luehrsen <ericluehrsen@hotmail.com>
-- Licensed to the public under the Apache License 2.0.

local m4, s4, frm
local filename = "/etc/unbound/unbound_ext.conf"
local description =  translatef("Here you may edit 'forward:' and 'remote-control:' in an extended 'include:'")
description = description .. " (" .. filename .. ")"

m4 = SimpleForm("editing", nil)
m4:append(Template("unbound/css-editing"))
m4.submit = translate("Save")
m4.reset = false
s4 = m4:section(SimpleSection, "Unbound Extended Conf", description)
frm = s4:option(TextValue, "data")
frm.datatype = "string"
frm.rows = 20


function frm.cfgvalue()
  return nixio.fs.readfile(filename) or ""
end


function frm.write(self, section, data)
  return nixio.fs.writefile(filename, luci.util.trim(data:gsub("\r\n", "\n")))
end


return m4

